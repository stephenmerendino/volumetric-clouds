#include "Engine/Core/Console.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Common.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Thread/critical_section.h"
#include "Engine/RHI/RHI.hpp"
#include "Engine/RHI/RHITexture2D.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Renderer/Font.hpp"
#include "Engine/Math/Matrix4.hpp"
#include "Engine/Config/EngineConfig.hpp"
#include "Engine/Net/remote_command_service.hpp"
#include <ctime>
#include <vector>
#include <map>
#include <stdarg.h>

// Default the toggle key to Tilde, but allow for overriding as well
#ifndef CONSOLE_TOGGLE_KEYCODE
#define CONSOLE_TOGGLE_KEYCODE '`'
#endif

#define BACKSPACE_KEYCODE 8
#define DELETE_KEYCODE 0x2E
#define HOME_KEYCODE 0x24
#define END_KEYCODE 0x23
#define ESCAPE_KEYCODE 27
#define ENTER_KEYCODE 13
#define SHIFT_KEYCODE 0x10
#define CONTROL_KEYCODE 0x11

#define LEFT_ARROW_KEYCODE 0x25
#define UP_ARROW_KEYCODE 0x26
#define RIGHT_ARROW_KEYCODE 0x27
#define DOWN_ARROW_KEYCODE 0x28

#define PAGE_UP_KEYCODE 0x21
#define PAGE_DOWN_KEYCODE 0x22

#define FONT_SCALE 0.02f
#define TEXT_LEFT_ALIGNMENT 0.007f
#define COMMAND_PROMPT_TEXT "dev:~$ "

#define STARTING_CONSOLE_LINE_Y_POS 0.992f
#define ENDING_CONSOLE_LINE_Y_POS 0.042f
#define NUM_LINES_VISIBLE (unsigned int)floor(((STARTING_CONSOLE_LINE_Y_POS - ENDING_CONSOLE_LINE_Y_POS) / FONT_SCALE))

#define CURSOR_BLINK_COOLDOWN_SECONDS 0.5f

static CriticalSection s_lock;
static Event<const std::string&> s_print_event;

//---------------------------------------------------------------------------------
CommandSelfRegister::CommandSelfRegister(const std::string& command_name, ConsoleFunction func, const std::string& desc)
{
	console_register_command(command_name, func, desc);
}



//---------------------------------------------------------------------------------
struct ConsoleCommand
{
	ConsoleCommand(){};
	ConsoleCommand(const std::string description, ConsoleFunction function)
		:m_description(description)
		, m_function(function)
	{}

	std::string m_description;
	ConsoleFunction m_function;
};

class CommandSystem
{
public:
	std::map<std::string, ConsoleCommand> m_commands;

public:
	CommandSystem();
	~CommandSystem();

	void register_command(const std::string& command_name, ConsoleFunction func, const std::string& desc = "");
	ConsoleCommand* find_command(const std::string& command_name);
	void run(const std::string& command_name, const std::string& args);
};



//---------------------------------------------------------------------------------
struct ConsoleLine
{
	std::string m_text;
	Rgba m_color;
	std::string m_timeStamp;
};

class ConsoleDisplay
{
public:
	bool						m_isOpen;
	Font&						m_font;
	std::vector<ConsoleLine>	m_consoleLog;
	std::vector<std::string>	m_enteredCommands;
	std::string					m_inputBuffer;

	RHITexture2D*				m_consoleTexture;

    CriticalSection             m_lock;

public:
	int m_commandHistoryOffset;
	int m_inputBufferCaretOffset;
	int m_consoleLogScrollOffset;

	int m_startSelectOffset;
	int m_endSelectOffset;
	bool m_isHighlighting;

	bool m_isShiftDown;
	bool m_isControlDown;

public:
	ConsoleDisplay(Font& font);
	~ConsoleDisplay();

	void SetFont(const Font& font);

	void Update(float deltaSeconds);
	void Render();

	void Show();
	void Hide();
	void ToggleVisibility();

	void Clear();

	inline bool IsOpen() const { return m_isOpen; }

	void RegisterCharKeyDown(char c);
	void RegisterNonCharKeyDown(unsigned char nonCharInput);
	void RegisterNonCharKeyUp(unsigned char nonCharInput);

	void PushConsoleLine(const Rgba& color, const std::string& text);

private:
	float m_cursorBlinkCooldown;
	bool m_cursorShowThisFrame;

private:
	void DrawBackground() const;
	void RenderConsoleLog() const;
	void RenderConsoleInput() const;
	void RenderCursor() const;
	void RenderConsoleInputHighlighting() const;
	void ResetCursorTimer();

	void ProcessInputBuffer();
	void ResetInputPrompt();
	void SetAspectCorrectOrtho() const;
	std::string GetCurrentTimeStamp() const;

	void RegisterBuiltInCommands();

	void ScrollConsoleLogUp(unsigned int linesToScroll = 10);
	void ScrollConsoleLogDown(unsigned int linesToScroll = 10);

	void UpdateHighlighting(int prevInputCaretPos, int newInputCaretPos);
	void DeleteHighlightedInputSelection();

	std::string GetSelectedInputText() const;
};


//---------------------------------------------------------------------------------
static ConsoleDisplay* s_console = nullptr;
static CommandSystem* s_command_system = nullptr;




ConsoleDisplay::ConsoleDisplay(Font& font)
	:m_font(font)
	,m_isOpen(false)
	,m_consoleTexture(nullptr)
	,m_cursorBlinkCooldown(0.0f)
	,m_cursorShowThisFrame(false)
	,m_commandHistoryOffset(0)
	,m_inputBufferCaretOffset(0)
	,m_consoleLogScrollOffset(0)
	,m_startSelectOffset(0)
	,m_endSelectOffset(0)
	,m_isShiftDown(false)
	,m_isControlDown(false)
	,m_isHighlighting(false)
{
	// Go ahead and reserve some memory for these
	m_consoleLog.reserve(500);
	m_enteredCommands.reserve(500);
	m_inputBuffer.reserve(500);

	m_consoleTexture = g_theRenderer->m_device->CreateRHITexture2DFromColor(Rgba::WHITE);

	RegisterBuiltInCommands();

    // read in entered commands from file
    LoadTextFileLinesToBuffer("Log/command.history", m_enteredCommands);
}

ConsoleDisplay::~ConsoleDisplay()
{
    // write out entered commands to file
    SaveTextFileLinesToBuffer("Log/command.history", m_enteredCommands);

    SAFE_DELETE(m_consoleTexture);
}

void ConsoleDisplay::SetFont(const Font& font)
{
	m_font = font;
}

void ConsoleDisplay::Update(float deltaSeconds)
{
	if(m_cursorBlinkCooldown <= 0.0f){
		m_cursorShowThisFrame = !m_cursorShowThisFrame;
		m_cursorBlinkCooldown = CURSOR_BLINK_COOLDOWN_SECONDS;
	}
	else{
		m_cursorBlinkCooldown -= deltaSeconds;
	}
}

void ConsoleDisplay::Render()
{
    SCOPE_LOCK(&m_lock);

	if(IsOpen()){
		g_theRenderer->SetModel(Matrix4::IDENTITY);

		float aspect = g_theRenderer->m_output->GetAspectRatio();
		Matrix4 consoleOrtho = RHIInstance::GetInstance().CreateOrthoProjection(0.0f, aspect, 0.0f, 1.0f);
		g_theRenderer->SetProjection(consoleOrtho);

		g_theRenderer->EnableDepth(false, false);
		g_theRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);

		DrawBackground();
		RenderConsoleLog();
		RenderConsoleInput();
	}
}

void ConsoleDisplay::DrawBackground() const
{
	g_theRenderer->SetView(Matrix4::IDENTITY);
	Matrix4 consoleOrtho = RHIInstance::GetInstance().CreateOrthoProjection(0.0f, 1.0f, 0.0f, 1.0f);
	g_theRenderer->SetProjection(consoleOrtho);

	g_theRenderer->SetShader(nullptr);
	g_theRenderer->SetTexture(m_consoleTexture);

	// Green Background
	g_theRenderer->DrawQuad2d(0.0f, 0.0f, 1.0f, 1.f, AABB2::ZERO_TO_ONE, CONSOLE_ACCENT_BG_COLOR);

	// Log
	g_theRenderer->DrawQuad2d(0.001f, 0.04f, 0.999f, 0.995f, AABB2::ZERO_TO_ONE, CONSOLE_MAIN_BG_COLOR);

	// Input
	g_theRenderer->DrawQuad2d(0.001f, 0.001f, 0.999f, 0.0375f, AABB2::ZERO_TO_ONE, CONSOLE_MAIN_BG_COLOR);
}

void ConsoleDisplay::RenderConsoleLog() const
{
	SetAspectCorrectOrtho();

	std::vector<ConsoleLine>::const_iterator start;
	std::vector<ConsoleLine>::const_iterator end;

	if(m_consoleLog.size() <= NUM_LINES_VISIBLE){
		start = m_consoleLog.begin();
		end = m_consoleLog.end();
	}
	else{
		start = m_consoleLog.end() - NUM_LINES_VISIBLE + m_consoleLogScrollOffset;
		end = start + NUM_LINES_VISIBLE;
	}

	float yPos = STARTING_CONSOLE_LINE_Y_POS;

	while(start != end){
		const ConsoleLine& line = *start;

		float timeStampWidth = m_font.GetTextWidth(line.m_timeStamp.data(), FONT_SCALE);

		g_theRenderer->DrawText2d(Vector2(TEXT_LEFT_ALIGNMENT, yPos), FONT_SCALE, line.m_color.GetScaledRGB(0.5f), line.m_timeStamp, m_font);
		g_theRenderer->DrawText2d(Vector2(TEXT_LEFT_ALIGNMENT + timeStampWidth, yPos), FONT_SCALE, line.m_color, line.m_text, m_font);

		yPos -= m_font.m_fontInfo->m_lineHeight * FONT_SCALE;
		++start;
	}

	if(m_consoleLogScrollOffset != 0){
		int linesBelowScreen = -m_consoleLogScrollOffset;
		std::string moreText = Stringf("%i...", linesBelowScreen);
		g_theRenderer->DrawText2d(Vector2(TEXT_LEFT_ALIGNMENT, yPos), FONT_SCALE, Rgba::WHITE.GetScaledRGB(0.5f), moreText.data(), m_font);
	}
}

void ConsoleDisplay::RenderConsoleInput() const
{
	SetAspectCorrectOrtho();
	
	g_theRenderer->DrawText2d(Vector2(TEXT_LEFT_ALIGNMENT, 0.03f), FONT_SCALE, Rgba::GREEN, "dev:~$ ", m_font);

	float promptOffset = m_font.GetTextWidth(COMMAND_PROMPT_TEXT, FONT_SCALE);

	g_theRenderer->DrawText2d(Vector2(TEXT_LEFT_ALIGNMENT + promptOffset, 0.03f), FONT_SCALE, Rgba::WHITE, m_inputBuffer, m_font);

	if(m_isHighlighting){
		RenderConsoleInputHighlighting();
	}
	else if(m_cursorShowThisFrame){
		RenderCursor();
	}
}

void ConsoleDisplay::RenderCursor() const
{
	float promptOffset = m_font.GetTextWidth(COMMAND_PROMPT_TEXT, FONT_SCALE);

	std::string inputBufferToCaret = std::string(m_inputBuffer.begin(), m_inputBuffer.end() + m_inputBufferCaretOffset);
	float inputBufferOffset = m_font.GetTextWidth(inputBufferToCaret, FONT_SCALE);

	float cursorMinX = TEXT_LEFT_ALIGNMENT + promptOffset + inputBufferOffset;
	float cursorMaxX = cursorMinX + 0.002f;

	float cursorMinY = 0.03f - (m_font.m_fontInfo->m_lineHeight * FONT_SCALE);
	float cursorMaxY = 0.03f;

	g_theRenderer->SetTexture(m_consoleTexture);
	g_theRenderer->DrawQuad2d(cursorMinX, cursorMinY, cursorMaxX, cursorMaxY, AABB2::ZERO_TO_ONE, Rgba::WHITE.GetScaledRGB(0.5f));
}

void ConsoleDisplay::RenderConsoleInputHighlighting() const
{
	float promptOffset = m_font.GetTextWidth(COMMAND_PROMPT_TEXT, FONT_SCALE);

	//g_theRenderer->DrawText2d(Vector2(TEXT_LEFT_ALIGNMENT + promptOffset, 0.03f), FONT_SCALE, Rgba::WHITE, m_font, m_inputBuffer);

	//width of the selected text
	int selectedTextStartIndex = m_inputBuffer.size() + m_startSelectOffset;
	int selectedTextEndIndex = m_inputBuffer.size() + m_endSelectOffset;

	std::string selectedText = GetSelectedInputText();
	float selectedTextWidth = m_font.GetTextWidth(selectedText, FONT_SCALE);

	//width of the text leading up to the selected text
	std::string textBeforeSelection = std::string(m_inputBuffer.begin(), m_inputBuffer.begin() + Min(selectedTextStartIndex, selectedTextEndIndex));
	float textBeforeSelectionWidth = m_font.GetTextWidth(textBeforeSelection, FONT_SCALE);

	Vector2 mins;
	Vector2 maxs;

	mins.x = TEXT_LEFT_ALIGNMENT + promptOffset + textBeforeSelectionWidth;
	maxs.x = mins.x + selectedTextWidth;

	maxs.y = 0.03f;
	mins.y = maxs.y - (m_font.m_fontInfo->m_lineHeight * FONT_SCALE);

	g_theRenderer->SetTexture(nullptr);
	g_theRenderer->DrawQuad2d(mins, maxs, AABB2::ZERO_TO_ONE, Rgba::WHITE);

	g_theRenderer->DrawText2d(Vector2(TEXT_LEFT_ALIGNMENT + promptOffset + textBeforeSelectionWidth, 0.03f), FONT_SCALE, Rgba::BLACK, selectedText, m_font);
}

void ConsoleDisplay::Show()
{
	m_isOpen = true;
    RemoteCommandService::get_instance()->set_visibility(true);
}

void ConsoleDisplay::Hide()
{
	m_isOpen = false;
    RemoteCommandService::get_instance()->set_visibility(false);
}

void ConsoleDisplay::ToggleVisibility()
{
	if(m_isOpen){
		Hide();
	}
	else{
		Show();
	}
}

void ConsoleDisplay::Clear()
{
    SCOPE_LOCK(&m_lock);
	m_consoleLog.clear();
	m_consoleLogScrollOffset = 0;
}

void ConsoleDisplay::RegisterCharKeyDown(char c)
{
	// Check for console toggle
	if(c == CONSOLE_TOGGLE_KEYCODE){
		ToggleVisibility();
		return;
	}
	
	// If not open, then don't allow any processing
	if(!IsOpen()){
		return;
	}

	// Handle backspace
	if(c == BACKSPACE_KEYCODE){
		m_cursorBlinkCooldown = CURSOR_BLINK_COOLDOWN_SECONDS;
		m_cursorShowThisFrame = true;

		if(!m_isHighlighting && m_inputBuffer.size() > 0 && -m_inputBufferCaretOffset < (int)m_inputBuffer.size()){
			std::string::iterator eraseEnd = m_inputBuffer.end() + m_inputBufferCaretOffset;
			std::string::iterator eraseStart = eraseEnd - 1;

			m_inputBuffer.erase(eraseStart, eraseEnd);
		}

		if(m_isHighlighting){
			DeleteHighlightedInputSelection();
		}

		ResetCursorTimer();
		return;
	}

	// Handle escape
	if(c == ESCAPE_KEYCODE){
		if(m_inputBuffer.size() > 0){
			ResetInputPrompt();
		}
		else{
			Hide();
		}

		return;
	}

	if(c == ENTER_KEYCODE){
		ProcessInputBuffer();
		return;
	}

	// Very, very hacky workaround for ctrl-c/ctrl-x/ctrl-v
	if(c == 22 || c == 3 || c == 24){
		return;
	}

	if(m_isHighlighting){
		DeleteHighlightedInputSelection();
	}

	// Push back any other chars into buffer
	m_inputBuffer.insert(m_inputBuffer.end() + m_inputBufferCaretOffset, c);

	ResetCursorTimer();
}

void ConsoleDisplay::ResetCursorTimer()
{
	m_cursorBlinkCooldown = CURSOR_BLINK_COOLDOWN_SECONDS;
	m_cursorShowThisFrame = true;
}

void ConsoleDisplay::UpdateHighlighting(int prevInputCaretPos, int newInputCaretPos)
{
	// Holding down shift
	if(m_isShiftDown){
		// Start highlighting
		if(!m_isHighlighting){
			m_isHighlighting = true;
			m_startSelectOffset = prevInputCaretPos;
			m_endSelectOffset = newInputCaretPos;
		}
		// Continue highlighting
		else if(m_isHighlighting){
			m_endSelectOffset = newInputCaretPos;
		}
	}
	// Not holding down shift
	else{
		// Was highlighting, now stahp
		if(m_isHighlighting){
			m_isHighlighting = false;
			m_startSelectOffset = 0;
			m_endSelectOffset = 0;
		}
	}
}

void ConsoleDisplay::DeleteHighlightedInputSelection()
{
	std::string::iterator eraseEnd = (m_inputBuffer.end()) + (m_endSelectOffset);
	std::string::iterator eraseStart = (m_inputBuffer.end()) + m_startSelectOffset;

	if(m_startSelectOffset > m_endSelectOffset){
		std::string::iterator temp = eraseStart;
		eraseStart = eraseEnd;
		eraseEnd = temp;
	}

	m_inputBuffer.erase(eraseStart, eraseEnd);

	m_inputBufferCaretOffset = Min(m_startSelectOffset, m_endSelectOffset) + abs(m_startSelectOffset - m_endSelectOffset);

	UpdateHighlighting(m_endSelectOffset, m_inputBufferCaretOffset);
}

void ConsoleDisplay::RegisterNonCharKeyDown(unsigned char nonCharInput)
{
	if(!IsOpen()){
		return;
	}

	// Movement within the input buffer
	if(nonCharInput == LEFT_ARROW_KEYCODE){
		int prevCaret = m_inputBufferCaretOffset;
		--m_inputBufferCaretOffset;
		m_inputBufferCaretOffset = Clamp(m_inputBufferCaretOffset, -((int)m_inputBuffer.size()), 0);

		UpdateHighlighting(prevCaret, m_inputBufferCaretOffset);
	}

	if(nonCharInput == RIGHT_ARROW_KEYCODE){
		int prevCaret = m_inputBufferCaretOffset;
		++m_inputBufferCaretOffset;
		m_inputBufferCaretOffset = Clamp(m_inputBufferCaretOffset, -((int)m_inputBuffer.size()), 0);

		UpdateHighlighting(prevCaret, m_inputBufferCaretOffset);
	}



	// Command History Cycling
	if(nonCharInput == UP_ARROW_KEYCODE){
		int numEnteredCommands = (int)m_enteredCommands.size();

		if(numEnteredCommands > m_commandHistoryOffset){
			m_inputBuffer = m_enteredCommands.at((numEnteredCommands - 1) - m_commandHistoryOffset);
			++m_commandHistoryOffset;
		}
	}

	if(nonCharInput == DOWN_ARROW_KEYCODE){
		m_commandHistoryOffset--;
		int numEnteredCommands = (int)m_enteredCommands.size();

		if(m_commandHistoryOffset > 0){
			m_inputBuffer = m_enteredCommands.at(numEnteredCommands - m_commandHistoryOffset);
		}
		else{
			ResetInputPrompt();
		}
	}


	if(nonCharInput == DELETE_KEYCODE){
		m_cursorBlinkCooldown = CURSOR_BLINK_COOLDOWN_SECONDS;
		m_cursorShowThisFrame = true;

		if(!m_isHighlighting && m_inputBuffer.size() > 0 && m_inputBufferCaretOffset < 0){
			std::string::iterator eraseEnd = m_inputBuffer.end() + (m_inputBufferCaretOffset + 1);
			std::string::iterator eraseStart = eraseEnd - 1;

			m_inputBuffer.erase(eraseStart, eraseEnd);
			m_inputBufferCaretOffset++;
		}

		if(m_isHighlighting){
			DeleteHighlightedInputSelection();
		}
	}

	if(nonCharInput == HOME_KEYCODE){
		int prevCaret = m_inputBufferCaretOffset;
		m_inputBufferCaretOffset = (int)m_inputBuffer.size() * -1;

		UpdateHighlighting(prevCaret, m_inputBufferCaretOffset);
	}

	if(nonCharInput == END_KEYCODE){
		int prevCaret = m_inputBufferCaretOffset;
		m_inputBufferCaretOffset = 0;

		UpdateHighlighting(prevCaret, m_inputBufferCaretOffset);
	}

	if(nonCharInput == PAGE_UP_KEYCODE){
		ScrollConsoleLogUp();
		return;
	}

	if(nonCharInput == PAGE_DOWN_KEYCODE){
		ScrollConsoleLogDown();
		return;
	}

	if(nonCharInput == SHIFT_KEYCODE){
		m_isShiftDown = true;
		return;
	}

	if(nonCharInput == CONTROL_KEYCODE){
		m_isControlDown = true;
		return;
	}

	// Copy
	if(nonCharInput == 'C' && m_isControlDown && m_isHighlighting){
		std::string selectedText = GetSelectedInputText();
		g_theRenderer->m_output->m_window->SetClipboard(selectedText);
		return;
	}

	// Cut
	if(nonCharInput == 'X' && m_isControlDown && m_isHighlighting){
		std::string selectedText = GetSelectedInputText();
		g_theRenderer->m_output->m_window->SetClipboard(selectedText);
		DeleteHighlightedInputSelection();
		return;
	}

	// Paste
	if(nonCharInput == 'V' && m_isControlDown){
		std::string clipboardText = g_theRenderer->m_output->m_window->GetClipboard();
		if(m_isHighlighting){
			DeleteHighlightedInputSelection();
		}
		m_inputBuffer.insert(m_inputBuffer.size() + m_inputBufferCaretOffset, clipboardText);
		return;
	}
	

	ResetCursorTimer();
}

void ConsoleDisplay::RegisterNonCharKeyUp(unsigned char nonCharInput)
{
	if(nonCharInput == SHIFT_KEYCODE){
		m_isShiftDown = false;
		return;
	}

	if(nonCharInput == CONTROL_KEYCODE){
		m_isControlDown = false;
		return;
	}
}

void ConsoleDisplay::PushConsoleLine(const Rgba& color, const std::string& text)
{
    SCOPE_LOCK(&m_lock);

	ConsoleLine newInputLine;
	newInputLine.m_text = text;
	newInputLine.m_color = color;
	newInputLine.m_timeStamp = GetCurrentTimeStamp();

	m_consoleLog.push_back(newInputLine);

    s_print_event.trigger(text);

	if(m_consoleLogScrollOffset != 0){
		m_consoleLogScrollOffset--;
	}
}

void ConsoleDisplay::ResetInputPrompt()
{
	// reset tracking vars
	m_commandHistoryOffset = 0;
	m_inputBufferCaretOffset = 0;
	m_inputBuffer.clear();
	m_isHighlighting = false;
	m_startSelectOffset = 0;
	m_endSelectOffset = 0;
}

void ConsoleDisplay::ProcessInputBuffer()
{
	m_consoleLogScrollOffset = 0;

	if(m_inputBuffer.size() == 0){
		PushConsoleLine(Rgba::WHITE, "");
		ResetInputPrompt();
		return;
	}
	else{
		m_enteredCommands.push_back(m_inputBuffer);
	}

	std::string commandName;
	std::string args;

	unsigned int firstWhitespacechar = 0;
	for(unsigned int charIndex = 0; charIndex < m_inputBuffer.size(); ++charIndex){
		if(m_inputBuffer[charIndex] == ' '){
			firstWhitespacechar = charIndex;
			break;
		}

		firstWhitespacechar = charIndex;
		commandName += m_inputBuffer[charIndex];
	}

	for(unsigned int charIndex = firstWhitespacechar + 1; charIndex < m_inputBuffer.size(); ++charIndex){
		args += m_inputBuffer[charIndex];
	}

	s_command_system->run(commandName, args);

	ResetInputPrompt();
}

void ConsoleDisplay::SetAspectCorrectOrtho() const
{
	float aspect = g_theRenderer->m_output->GetAspectRatio();
	Matrix4 consoleOrtho = RHIInstance::GetInstance().CreateOrthoProjection(0.0f, aspect, 0.0f, 1.0f);
	g_theRenderer->SetProjection(consoleOrtho);
}

static void ClearCommand(ConsoleArgs& args)
{
	console_clear();
}

static void HelpCommand(ConsoleArgs& args)
{
	for(std::pair<const std::string, ConsoleCommand> commandPair : s_command_system->m_commands){
		ConsoleCommand command = commandPair.second;

		std::string output;
		output += commandPair.first;
		output += ": ";
		output += command.m_description;

		console_printf(Rgba::WHITE, output);
	}
}

static void ExitCommand(ConsoleArgs& args)
{
	s_console->Hide();
	s_console->Clear();
}

static void SaveConsoleCommand(ConsoleArgs& args)
{
	std::string filepath = args.next_string_arg();
	if(filepath.size() == 0){
		console_printf(Rgba::RED, "error: cannot save out console. \"%s\" is an invalid filepath", filepath.data());
		return;
	}

	console_printf(Rgba::YELLOW, "saving console to %s...", filepath.data());

	std::string output;

	for(ConsoleLine& line : s_console->m_consoleLog){
		output += line.m_timeStamp + line.m_text + "\n";
	}

	output += '\0';

	bool saved = SaveBufferToBinaryFile(output.data(), filepath.data());
	if(!saved){
		console_printf(Rgba::RED, "error: problem occurred while writing log to file\"%s\"", filepath.data());
		return;
	}

	console_printf(Rgba::GREEN, "saved!");
}

void ConsoleDisplay::RegisterBuiltInCommands()
{
	console_register_command("clear", ClearCommand, "clears the developer console");
	console_register_command("help", HelpCommand, "shows all registered console commands");
	console_register_command("exit", ExitCommand, "exits the developer console");
	console_register_command("save_console", SaveConsoleCommand, "[string:filepath] :saves the console log out to a file. Spaces not currently supported in filepath.");
}

std::string ConsoleDisplay::GetCurrentTimeStamp() const
{
	// Build date/time string
	std::time_t rawTime = std::time(nullptr);
	tm localTime;
	localtime_s(&localTime, &rawTime);
	char timeString[15];
	strftime(timeString, 15, "[%r] \0", &localTime);
	return std::string(timeString);
}

void ConsoleDisplay::ScrollConsoleLogUp(unsigned int linesToScroll)
{
	if(m_consoleLog.size() + m_consoleLogScrollOffset < NUM_LINES_VISIBLE){
		return;
	}

	m_consoleLogScrollOffset -= linesToScroll;

	int maxOffset = -1 * (int)(m_consoleLog.size() - NUM_LINES_VISIBLE);

	m_consoleLogScrollOffset = Max(m_consoleLogScrollOffset, maxOffset);
}

void ConsoleDisplay::ScrollConsoleLogDown(unsigned int linesToScroll)
{
	m_consoleLogScrollOffset += linesToScroll;
	m_consoleLogScrollOffset = Min(m_consoleLogScrollOffset, 0);
}

std::string ConsoleDisplay::GetSelectedInputText() const
{
	int selectedTextStartIndex = m_inputBuffer.size() + m_startSelectOffset;
	int selectedTextEndIndex = m_inputBuffer.size() + m_endSelectOffset;

	if(selectedTextStartIndex > selectedTextEndIndex){
		int temp = selectedTextStartIndex;
		selectedTextStartIndex = selectedTextEndIndex;
		selectedTextEndIndex = temp;
	}

	std::string selectedText = std::string(m_inputBuffer.begin() + selectedTextStartIndex, m_inputBuffer.begin() + selectedTextEndIndex);
	return selectedText;
}



//---------------------------------------------------------------------------------
CommandSystem::CommandSystem()
{
}

CommandSystem::~CommandSystem()
{
}

void CommandSystem::register_command(const std::string& command_name, ConsoleFunction func, const std::string& desc)
{
	if(find_command(command_name)){
		console_warning("Console command registered twice: " + command_name);
	}
	else{
		m_commands[command_name] = ConsoleCommand(desc, func);
	}
}

ConsoleCommand* CommandSystem::find_command(const std::string& command_name)
{
	auto iter = m_commands.find(command_name);
	if(iter != m_commands.end()) {
		return &iter->second;
	}
	else {
		return nullptr;
	}
}

void CommandSystem::run(const std::string& command_name, const std::string& args)
{
	ConsoleCommand* command = find_command(command_name);

	if(command){
		console_success(command_name + " " + args);
		ConsoleArgs console_args(args);
        SCOPE_LOCK(&s_console->m_lock); // make sure that we only allow further printing to the console from this command
		command->m_function(console_args);
	}
	else{
		console_error("Command %s not found!", command_name.c_str());
	}
}



//---------------------------------------------------------------------------------
ConsoleArgs::ConsoleArgs()
	:m_raw_args("")
	,m_cursor(0)
{
}

ConsoleArgs::ConsoleArgs(const std::string& raw_args)
	:m_raw_args(raw_args)
	,m_cursor(0)
{
}

std::string ConsoleArgs::next_string_arg()
{
	advance_to_next_token();

	if(is_at_end()){
		console_warning("A console command tried to get a string arg, but prematurely reached the end");
		return "";
	}

	unsigned int token_start = m_cursor;
	unsigned int token_end = token_start;

	if(m_raw_args[token_start] == '\"'){
		++m_cursor;
		token_start = m_cursor;

		while(m_raw_args[m_cursor] != '\"' && !is_at_end()){
			++m_cursor;
		}

		token_end = m_cursor;
		m_cursor++;
	}
	else{
		while(m_raw_args[m_cursor] != ' ' && m_raw_args[m_cursor] != ',' && !is_at_end()){
			++m_cursor;
		}

		token_end = m_cursor;
	}

	return std::string(m_raw_args.begin() + token_start, m_raw_args.begin() + token_end);
}

unsigned int ConsoleArgs::next_uint_arg()
{
	return (unsigned int)next_int_arg();
}

int ConsoleArgs::next_int_arg()
{
	advance_to_next_token();

	if(is_at_end()){
		console_warning("A console command tried to get an int arg, but prematurely reached the end");
		return 0;
	}

	std::string arg_as_string = next_string_arg();
	return atoi(arg_as_string.c_str());
}

float ConsoleArgs::next_float_arg()
{
	advance_to_next_token();

	if(is_at_end()){
		console_warning("A console command tried to get a float arg, but prematurely reached the end");
		return 0.0f;
	}

	std::string arg_as_string = next_string_arg();
	return (float)atof(arg_as_string.c_str());
}

bool ConsoleArgs::next_bool_arg()
{
	advance_to_next_token();

	if(is_at_end()){
		console_warning("A console command tried to get a bool arg, but prematurely reached the end. Returning false.");
		return false;
	}

	std::string arg_as_string = next_string_arg();

	if(arg_as_string == "true" || arg_as_string == "TRUE"){
		return true;
	}

	if(arg_as_string == "false" || arg_as_string == "false"){
		return false;
	}

	console_error("Could not parse console argument to bool. Returning false.");
	return false;
}

void ConsoleArgs::advance_to_next_token()
{
	if(is_at_end()){
		return;
	}

	while(m_raw_args[m_cursor] == ' ' || m_raw_args[m_cursor] == ','){
		++m_cursor;

		if(is_at_end()){
			break;
		}
	}
}

std::string ConsoleArgs::get_remaining_args_as_string()
{
    advance_to_next_token();
    return std::string(m_raw_args.begin() + m_cursor, m_raw_args.end());
}

bool ConsoleArgs::is_at_end()
{
	return m_cursor == m_raw_args.size();
}



//---------------------------------------------------------------------------------
void console_init(Font& font)
{
    if(nullptr == s_command_system){
    	s_command_system = new CommandSystem();
    }
    if(nullptr == s_console){
    	s_console = new ConsoleDisplay(font);
    }
}

void console_shutdown()
{
    SCOPE_LOCK(&s_lock);
	SAFE_DELETE(s_command_system);
	SAFE_DELETE(s_console);
}

void console_update(float delta_seconds)
{
	s_console->Update(delta_seconds);
}

void console_set_font(const Font& font)
{
	s_console->SetFont(font);
}

void console_render()
{
    SCOPE_LOCK(&s_lock);
	s_console->Render();
}

void console_show()
{
	s_console->Show();
}

void console_hide()
{
	s_console->Hide();
}

void console_toggle_visibility()
{
	s_console->ToggleVisibility();
}

void console_clear()
{
    SCOPE_LOCK(&s_lock);
	s_console->Clear();
}

bool is_console_open()
{
	return s_console->IsOpen();
}

void console_register_char_key_down(char c)
{
	s_console->RegisterCharKeyDown(c);
}

void console_register_non_char_key_down(unsigned char uc)
{
	s_console->RegisterNonCharKeyDown(uc);
}

void console_register_non_char_key_up(unsigned char uc)
{
	s_console->RegisterNonCharKeyUp(uc);
}

void console_printf(const Rgba& color, const char* format, ...)
{
    SCOPE_LOCK(&s_lock);
	const int MESSAGE_MAX_LENGTH = 2048;
	char msg_literal[MESSAGE_MAX_LENGTH];
	va_list arg_list;
	va_start(arg_list, format);
	vsnprintf_s(msg_literal, MESSAGE_MAX_LENGTH, _TRUNCATE, format, arg_list);
	va_end(arg_list);
	msg_literal[MESSAGE_MAX_LENGTH - 1] = '\0';

	if(s_console){
		s_console->PushConsoleLine(color, msg_literal);
	}
	else{
		printf("%s\n", msg_literal);
	}
}

void console_printf(const Rgba& color, const std::string& text)
{
    SCOPE_LOCK(&s_lock);
	if(s_console){
		s_console->PushConsoleLine(color, text);
	}
	else{
		printf("%s\n", text.c_str());
	}
}

void console_info(const char* format, ...)
{
    SCOPE_LOCK(&s_lock);
	const int MESSAGE_MAX_LENGTH = 2048;
	char msg_literal[MESSAGE_MAX_LENGTH];
	va_list arg_list;
	va_start(arg_list, format);
	vsnprintf_s(msg_literal, MESSAGE_MAX_LENGTH, _TRUNCATE, format, arg_list);
	va_end(arg_list);
	msg_literal[MESSAGE_MAX_LENGTH - 1] = '\0';

	if(s_console){
		s_console->PushConsoleLine(Rgba::WHITE, msg_literal);
	}
	else{
		printf("%s\n", msg_literal);
	}
}

void console_info(const std::string& text)
{
	console_printf(Rgba::WHITE, text);
}

void console_warning(const char* format, ...)
{
    SCOPE_LOCK(&s_lock);
	const int MESSAGE_MAX_LENGTH = 2048;
	char msg_literal[MESSAGE_MAX_LENGTH];
	va_list arg_list;
	va_start(arg_list, format);
	vsnprintf_s(msg_literal, MESSAGE_MAX_LENGTH, _TRUNCATE, format, arg_list);
	va_end(arg_list);
	msg_literal[MESSAGE_MAX_LENGTH - 1] = '\0';

	console_printf(Rgba::YELLOW, msg_literal);
}

void console_warning(const std::string& text)
{
	console_printf(Rgba::YELLOW, text);
}

void console_error(const char* format, ...)
{
    SCOPE_LOCK(&s_lock);
	const int MESSAGE_MAX_LENGTH = 2048;
	char msg_literal[MESSAGE_MAX_LENGTH];
	va_list arg_list;
	va_start(arg_list, format);
	vsnprintf_s(msg_literal, MESSAGE_MAX_LENGTH, _TRUNCATE, format, arg_list);
	va_end(arg_list);
	msg_literal[MESSAGE_MAX_LENGTH - 1] = '\0';

	console_printf(Rgba::RED, msg_literal);
}

void console_error(const std::string& text)
{
	console_printf(Rgba::RED, text);
}

void console_success(const char* format, ...)
{
    SCOPE_LOCK(&s_lock);
	const int MESSAGE_MAX_LENGTH = 2048;
	char msg_literal[MESSAGE_MAX_LENGTH];
	va_list arg_list;
	va_start(arg_list, format);
	vsnprintf_s(msg_literal, MESSAGE_MAX_LENGTH, _TRUNCATE, format, arg_list);
	va_end(arg_list);
	msg_literal[MESSAGE_MAX_LENGTH - 1] = '\0';

	console_printf(Rgba::GREEN, msg_literal);
}

void console_success(const std::string& text)
{
	console_printf(Rgba::GREEN, text);
}

void console_register_command(const std::string& command_name, ConsoleFunction func, const std::string& desc)
{
	if(!s_command_system){
		s_command_system = new CommandSystem();
	}

	s_command_system->register_command(command_name, func, desc);
}

void console_run_command(const std::string& command_name)
{
	if(!s_command_system){
		s_command_system = new CommandSystem();
	}

	s_command_system->run(command_name, "");
}

void console_run_command_and_args(const std::string& command_and_args)
{
	if(!s_command_system){
		s_command_system = new CommandSystem();
	}

    size_t split =command_and_args.find_first_of(' ');

    if(split == std::string::npos){
        s_command_system->run(command_and_args, "");
        return;
    }

    std::string command_name = command_and_args.substr(0, split);
    std::string command_args = command_and_args.substr(split + 1);

	s_command_system->run(command_name, command_args);
}

void console_register_to_print_event(void* user_arg, print_cb cb)
{
    s_print_event.subscribe(user_arg, cb);
}

void console_unregister_to_print_event(void* user_arg, print_cb cb)
{
    s_print_event.unsubscribe(user_arg, cb);
}