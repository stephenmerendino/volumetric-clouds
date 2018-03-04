#include "Engine/Renderer/Motion.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

Motion::Motion()
	:m_name("unnamed motion")
	,m_framerate(0)
	,m_duration_seconds(0)
{
}

Motion::Motion(const std::string& name)
	:m_name(name)
{
}

Motion::Motion(const std::string& name, float duration_seconds, float frames_per_second)
	:m_name(name)
	,m_framerate(frames_per_second)
{
	set_duration_seconds(duration_seconds);
}

void Motion::set_name(const std::string& name)
{
	m_name = name;
}

std::string Motion::get_name() const
{
	return m_name;
}

void Motion::set_duration_seconds(float seconds)
{
	m_duration_seconds = seconds;

	int num_poses = (int)ceil(m_duration_seconds * m_framerate) + 1;
	m_poses.resize(num_poses);
}

float Motion::get_duration_seconds() const
{
	return m_duration_seconds;
}

void Motion::set_framerate(float frames_per_second)
{
	m_framerate = frames_per_second;
}

float Motion::get_framerate() const
{
	return m_framerate;
}

void Motion::resize_poses(unsigned int num_joints)
{
	for(Pose& p : m_poses){
		p.m_local_transforms.resize(num_joints);
	}
}

void Motion::add_pose(const Pose& pose, int index)
{
	m_poses[index] = pose;
}

Pose* Motion::get_pose(unsigned int frame_index)
{
	return &m_poses[frame_index];
}

unsigned int Motion::get_frame_count() const
{
	return m_poses.size();
}

void Motion::evaluate(Pose* out_pose, float time_in_seconds) const
{
	float time_between_frames = 1.0f / m_framerate;

	// Loop it 
	time_in_seconds = fmodf(time_in_seconds, m_duration_seconds);

	int start_pose_index = (int)floor(time_in_seconds * m_framerate);
	int end_pose_index = start_pose_index + 1;

	float t = (time_in_seconds - ((float)start_pose_index * time_between_frames)) / time_between_frames;

	const Pose& start_pose = m_poses[start_pose_index];
	const Pose& end_pose = m_poses[end_pose_index];

	// Make sure we have enough space
	out_pose->m_local_transforms.resize(start_pose.m_local_transforms.size());

	// Interpolate each joint's local transform seperately
	for(size_t i = 0; i < start_pose.m_local_transforms.size(); ++i){
		out_pose->m_local_transforms[i] = nlerp(start_pose.m_local_transforms[i], end_pose.m_local_transforms[i], t);
	}
}

//- *.motion:  A single motion.  At minimum should store off framerate, duration, frame count, and 
//  the array of poses.  Each pose is just an array of local transforms.  
bool Motion::write(BinaryStream& stream)
{
	stream.m_stream_order = LITTLE_ENDIAN;

	ASSERT_OR_DIE(stream.write(m_framerate), "Failed to write frame");
	ASSERT_OR_DIE(stream.write(m_duration_seconds), "Failed to write duration");
	ASSERT_OR_DIE(stream.write(m_poses.size()), "Failed to write poses size");

	for(const Pose& pose : m_poses){
		ASSERT_OR_DIE(stream.write(pose), "Failed to write pose");
	}

	return true;
}

bool Motion::read(BinaryStream& stream)
{
	stream.m_stream_order = LITTLE_ENDIAN;

	ASSERT_OR_DIE(stream.read(m_framerate), "Failed to read framerate");
	ASSERT_OR_DIE(stream.read(m_duration_seconds), "Failed to read duration seconds");

	unsigned int num_poses;
	ASSERT_OR_DIE(stream.read(num_poses), "Failed to read num poses");

	m_poses.resize(num_poses);
	Pose pose;

	for(unsigned int index = 0; index < num_poses; ++index){
		ASSERT_OR_DIE(stream.read(pose), "Failed to read pose");
		m_poses[index] = pose;
	}

	return true;
}