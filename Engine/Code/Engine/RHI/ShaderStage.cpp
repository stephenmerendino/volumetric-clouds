#include "Engine/RHI/ShaderStage.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/Renderer/Vertex3.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Common.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//-----------------------------------
// Internal helpers
//-----------------------------------

const char* shader_input_typename(D3D_SHADER_INPUT_TYPE shader_input_type)
{
	switch(shader_input_type){
		case D3D_SIT_CBUFFER: return "constant buffer";
	    case D3D_SIT_TBUFFER: return "texture buffer";
	    case D3D_SIT_TEXTURE: return "texture";
	    case D3D_SIT_SAMPLER: return "sampler";
	    case D3D_SIT_UAV_RWTYPED: return "read-and-write buffer";
	    case D3D_SIT_STRUCTURED: return "structured buffer";
	    case D3D_SIT_UAV_RWSTRUCTURED: return "read-and-write structured buffer";
	    case D3D_SIT_BYTEADDRESS: return "byte-address buffer";
	    case D3D_SIT_UAV_RWBYTEADDRESS: return "read-and-write byte-address buffer";
	    case D3D_SIT_UAV_APPEND_STRUCTURED: return "append-structured buffer";
	    case D3D_SIT_UAV_CONSUME_STRUCTURED: return "consume-structured buffer";
	    case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER: return "read-and-write structured buffer with counter";
		default: return "unknown shader input type";
	}
}

RenderDataFormat get_render_format_for_shader_input_type(D3D_SHADER_INPUT_TYPE sit)
{
	switch(sit){
		case D3D_SIT_CBUFFER: return RENDER_DATA_FORMAT_CONSTANT_BUFFER;
	    case D3D_SIT_TBUFFER: return RENDER_DATA_FORMAT_TEXTURE_BUFFER;
	    case D3D_SIT_TEXTURE: return RENDER_DATA_FORMAT_TEXTURE;
	    case D3D_SIT_SAMPLER: return RENDER_DATA_FORMAT_SAMPLER;
	    case D3D_SIT_UAV_RWTYPED: return RENDER_DATA_FORMAT_RW_BUFFER;
	    case D3D_SIT_STRUCTURED: return RENDER_DATA_FORMAT_STRUCTURED_BUFFER;
	    case D3D_SIT_UAV_RWSTRUCTURED: return RENDER_DATA_FORMAT_RW_STRUCTURED_BUFFER;
	    case D3D_SIT_BYTEADDRESS: return RENDER_DATA_FORMAT_BYTE_ADDRESS_BUFFER;
	    case D3D_SIT_UAV_RWBYTEADDRESS: return RENDER_DATA_FORMAT_RW_BYTE_ADDRESS_BUFFER;
	    case D3D_SIT_UAV_APPEND_STRUCTURED: return RENDER_DATA_FORMAT_APPEND_STRUCTURED_BUFFER;
	    case D3D_SIT_UAV_CONSUME_STRUCTURED: return RENDER_DATA_FORMAT_CONSUME_STRUCTURED_BUFFER;
	    case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER: return RENDER_DATA_FORMAT_RW_STRUCTURED_BUFFER_WITH_COUNTER;
		default: return RENDER_DATA_FORMAT_UNKNOWN;
	}
}

const char* constant_buffer_typename(D3D_CBUFFER_TYPE cbuffer_type)
{
	switch(cbuffer_type){
		case D3D_CT_CBUFFER: return "scalars";
		case D3D_CT_TBUFFER: return "texture data";
		case D3D_CT_INTERFACE_POINTERS: return "interface pointers";
		case D3D_CT_RESOURCE_BIND_INFO: return "binding info";
		default: return "unkonwn type";
	}
}

//-----------------------------------
// ShaderStage
//-----------------------------------

ShaderStage::ShaderStage(const char* shader_raw_source, const size_t shader_raw_source_size, const char* entry_point, const char* target)
	:m_byte_code(nullptr)
{
	compile_shader_source_to_blob(nullptr, shader_raw_source, shader_raw_source_size, entry_point, target);
	//print_constants();
	cache_shader_resources();
}

ShaderStage::~ShaderStage()
{
	DX_SAFE_RELEASE(m_byte_code);
}

void ShaderStage::compile_shader_source_to_blob(const char* opt_filename, 
											    const void* source_code, 
											    const size_t source_code_size, 
											    const char* entry_point, 
											    const char* target)
{
   DWORD compile_flags = 0U;
   #if defined(DEBUG_SHADERS)
      compile_flags |= D3DCOMPILE_DEBUG;
      compile_flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
      compile_flags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
   #else 
      // compile_flags |= D3DCOMPILE_SKIP_VALIDATION;
      compile_flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
   #endif

   ID3DBlob *errors = nullptr;

   HRESULT hr = ::D3DCompile( source_code, 
      source_code_size,                   
      opt_filename,                       
      nullptr,                            
      D3D_COMPILE_STANDARD_FILE_INCLUDE,  
      entry_point,                         
      target,                             
      compile_flags,                      
      0,                                  
      &m_byte_code,                       
      &errors );                          

   if (FAILED(hr) || (errors != nullptr)) {
      if (errors != nullptr) {
         char *error_string = (char*) errors->GetBufferPointer();
         DebuggerPrintf( "Failed to compile [%s].  Compiler gave the following output;\n%s", 
            opt_filename, 
            error_string );
         DX_SAFE_RELEASE(errors);
      }
   } 

   ASSERT_OR_DIE(m_byte_code != nullptr, "Shader byte code compilation failed.\n");
}

void ShaderStage::print_constants()
{
   if (!m_byte_code) {
      return;
   }

   DebuggerPrintf("-------------Shader Trace------------\n");
 
   ID3D11ShaderReflection* reflector = nullptr;
   ::D3DReflect( m_byte_code->GetBufferPointer(),
      m_byte_code->GetBufferSize(),
      IID_ID3D11ShaderReflection,
      (void**) &reflector );
 
   // First off, let's list off all the resources
   unsigned int ri = 0;
   D3D11_SHADER_INPUT_BIND_DESC res_desc;
 
   while (SUCCEEDED(reflector->GetResourceBindingDesc( ri, &res_desc ))) {
      DebuggerPrintf("Resource [%s]. type[%s], bind[%u:%u]\n",
         res_desc.Name,
         shader_input_typename(res_desc.Type),
         res_desc.BindPoint,
         res_desc.BindCount );
 
      ++ri;
   }
 
   // Next, list all the constant buffers
   for (UINT i = 0; i < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT; ++i) {
      ID3D11ShaderReflectionConstantBuffer *buf = reflector->GetConstantBufferByIndex(i);
      if (nullptr != buf) {
         D3D11_SHADER_BUFFER_DESC desc;
         if (SUCCEEDED(buf->GetDesc(&desc))) {
            DebuggerPrintf("%s Constant Buffer [%u:%s] (%uB) has %u variables;\n",
               constant_buffer_typename(desc.Type),
               i,
               desc.Name,
               desc.Size,
               desc.Variables );
 
           unsigned int cur_offset = 0U;
            for (UINT vi = 0; vi < desc.Variables; ++vi) {
               ID3D11ShaderReflectionVariable *var = buf->GetVariableByIndex(vi);

			   ID3D11ShaderReflectionType* var_type = var->GetType();
			   D3D11_SHADER_TYPE_DESC var_type_desc;
			   var_type->GetDesc(&var_type_desc);

               if (nullptr != var) {
                  D3D11_SHADER_VARIABLE_DESC var_desc;
                  if (SUCCEEDED(var->GetDesc(&var_desc))) {
                     DebuggerPrintf("  Contains[%s,%s] offset[%u], size[%u].\n",
                        var_desc.Name,
						var_type_desc.Name,
                        cur_offset,
                        var_desc.Size );
                  }
 
                  cur_offset += var_desc.Size;
               }
            }
         }
      }
   }
 
   DX_SAFE_RELEASE(reflector);
}

void ShaderStage::cache_shader_resources()
{
	ID3D11ShaderReflection* reflector = nullptr;
	D3DReflect(m_byte_code->GetBufferPointer(),
			   m_byte_code->GetBufferSize(),
			   IID_ID3D11ShaderReflection,
			   (void**) &reflector);
 
   unsigned int ri = 0;
   D3D11_SHADER_INPUT_BIND_DESC res_desc;
 
	while (SUCCEEDED(reflector->GetResourceBindingDesc( ri, &res_desc ))) {
		shader_resource_t shader_res;
		shader_res.name = res_desc.Name;
		shader_res.data_format = get_render_format_for_shader_input_type(res_desc.Type);
		shader_res.bind_index = res_desc.BindPoint;
		shader_res.bind_count = res_desc.BindCount;

		m_shader_resources.push_back(shader_res);

		++ri;
	}

	// save off property blocks

	DX_SAFE_RELEASE(reflector);
}

int ShaderStage::find_bind_index_for_name(const char* bind_name)
{
	for(const shader_resource_t& res : m_shader_resources){
		if(res.name == bind_name){
			return res.bind_index;
		}
	}

	return INVALID_BIND_INDEX;
}