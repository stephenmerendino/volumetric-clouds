#pragma once

#include "Engine/Renderer/Pose.hpp"

#include <vector>
#include <string>

class Motion
{
public:
	std::string m_name;
	float m_duration_seconds;
	float m_framerate;
	std::vector<Pose> m_poses;

public:
	Motion();
	Motion(const std::string& name);
	Motion(const std::string& name, float duration_seconds, float frames_per_second);

	void set_name(const std::string& name);
	std::string get_name() const;

	void set_duration_seconds(float seconds);
	float get_duration_seconds() const;

	void set_framerate(float frames_per_second);
	float get_framerate() const;

	unsigned int get_frame_count() const;

	void resize_poses(unsigned int num_joints);
	void add_pose(const Pose& pose, int index);
	Pose* get_pose(unsigned int frame_index);

	void evaluate(Pose* out_pose, float time_in_seconds) const;

	bool write(BinaryStream& stream);
	bool read(BinaryStream& stream);
};