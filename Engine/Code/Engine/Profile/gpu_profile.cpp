#include "Engine/Profile/gpu_profile.h"
#include "Engine/RHI/RHIDeviceContext.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/DX11.hpp"
#include "Engine/Core/log.h"
#include "Engine/Thread/thread.h"
#include "Engine/Core/ErrorWarningAssert.hpp"

static ID3D11Query* g_disjoint_query = nullptr;
static ID3D11Query* g_pre_query = nullptr;
static ID3D11Query* g_post_query = nullptr;

static ID3D11Query* create_query(RHIDevice *device, D3D11_QUERY query_type)
{
    D3D11_QUERY_DESC desc;
    ZeroMemory(&desc, sizeof(desc));

    desc.Query = query_type;
    desc.MiscFlags = 0;        // some query types require flags, but none we're using.

    ID3D11Query *dx_query = nullptr;
    HRESULT hr = device->m_dxDevice->CreateQuery(&desc, &dx_query);

    if(FAILED(hr)) {
        log_errorf("Failed to create query!");
    }

    return dx_query;
}

void gpu_profile_init(RHIDevice* device)
{
    g_disjoint_query = create_query(device, D3D11_QUERY_TIMESTAMP_DISJOINT);
    g_pre_query = create_query(device, D3D11_QUERY_TIMESTAMP);
    g_post_query = create_query(device, D3D11_QUERY_TIMESTAMP);
}

void gpu_query_start_frame(RHIDeviceContext* context)
{
    if(nullptr != g_disjoint_query){
        context->m_dxDeviceContext->Begin(g_disjoint_query);
    }
}

double gpu_query_end_frame(RHIDeviceContext* context)
{
    if(nullptr == g_disjoint_query){
        return -1.0f;
    }

    context->m_dxDeviceContext->End(g_disjoint_query);

    // Now, spin until the query has data
    D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjoint_data;

    // hack to wait until I have data.
    HRESULT hr = S_FALSE;
    while(hr == S_FALSE) {
        hr = context->m_dxDeviceContext->GetData(g_disjoint_query, &disjoint_data, sizeof(disjoint_data), 0);
        thread_yield();
    }

    // is the data valid?
    if(disjoint_data.Disjoint == FALSE) {
        // if this has data, everything else should too
        UINT64 begin_time;
        UINT64 end_time;

        hr = context->m_dxDeviceContext->GetData(g_pre_query, &begin_time, sizeof(begin_time), 0);
        ASSERT_OR_DIE(SUCCEEDED(hr), "Failed to get data from pre query");

        hr = context->m_dxDeviceContext->GetData(g_post_query, &end_time, sizeof(end_time), 0);
        ASSERT_OR_DIE(SUCCEEDED(hr), "Failed to get data from post query");

        UINT64 elapsed = end_time - begin_time;

        // update in hz, so counters per second
        // disjoint_data.Frequency

        double seconds = (double)elapsed / (double)disjoint_data.Frequency;
        double ms = 1000.0 * seconds;
        // ms is how long the mandelbrot took - print it on the screen for the points;

        return ms;
    }

    return -1.0f;
}

void gpu_query_begin(RHIDeviceContext* context)
{
    if(nullptr != g_pre_query){
        context->m_dxDeviceContext->End(g_pre_query);
    }
}

void gpu_query_end(RHIDeviceContext* context)
{
    if(nullptr != g_post_query){
        context->m_dxDeviceContext->End(g_post_query);
    }
}