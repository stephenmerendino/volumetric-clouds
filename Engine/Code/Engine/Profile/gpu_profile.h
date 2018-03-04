#pragma once

class RHIDevice;
class RHIDeviceContext;

void gpu_profile_init(RHIDevice* device);

void gpu_query_start_frame(RHIDeviceContext* context);
double gpu_query_end_frame(RHIDeviceContext* context);

void gpu_query_begin(RHIDeviceContext* context);
void gpu_query_end(RHIDeviceContext* context);