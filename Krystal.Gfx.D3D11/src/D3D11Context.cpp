#include "Krystal.Gfx.D3D11/D3D11Context.hpp"

#pragma comment(lib, "d3dcompiler.lib")
#include <d3d11.h>
#include <d3dcompiler.h>

#include <cassert>

namespace
{
  static const std::string hlslSource = "struct VOut\n"
                                        "{\n"
                                        "    float4 pos : SV_POSITION;\n"
                                        "};\n"
                                        "VOut VShader(float4 position : POSITION)\n"
                                        "{\n"
                                        "    VOut o;\n"
                                        "    o.pos = position;\n"
                                        "    return o;\n"
                                        "}\n"
                                        "float4 PShader(VOut input) : SV_TARGET\n"
                                        "{\n"
                                        "    // orange\n"
                                        "    return float4(0.8f, 0.25f, 0.82f, 1.0f);\n"
                                        "}\n";
  static float vertices[] = {0.0f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f};
  static ID3D11VertexShader *pVS;
  static ID3D11PixelShader *pPS;
  static ID3D11Buffer *pVBuffer;
  static ID3D11InputLayout *pLayout;
}

namespace Krys::Gfx
{
  Expected<Unique<IContext>> CreateContext(NativeHandle windowHandle, uint32 width, uint32 height) noexcept
  {
    try
    {
      return Expected<Unique<IContext>>(CreateUnique<D3D11::D3D11Context>(windowHandle, width, height));
    }
    catch (const std::exception &e)
    {
      return Unexpected(e.what());
    }
  }
}

namespace Krys::Gfx::D3D11
{
  D3D11Context::D3D11Context(NativeHandle windowHandle, uint32 width, uint32 height)
      : _windowHandle(windowHandle.As<HWND>())
  {
    ::DXGI_SWAP_CHAIN_DESC scd {};
    scd.BufferCount = 1;                                // one back buffer
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // use 32-bit color
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;  // how swap chain is to be used
    scd.OutputWindow = _windowHandle;                   // the window to be used
    scd.SampleDesc.Count = 1;                           // how many multisamples
    scd.Windowed = TRUE;                                // windowed/full-screen mode
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // allow full-screen switching

    // create a device, device context and swap chain using the information in the scd struct
    ::D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, NULL,
                                    NULL, D3D11_SDK_VERSION, &scd, &_swapchain, &_device, NULL, &_context);

    // get the address of the back buffer
    ::ID3D11Texture2D *pBackBuffer {};
    _swapchain->GetBuffer(0, __uuidof(::ID3D11Texture2D), (LPVOID *)&pBackBuffer);
    if (!pBackBuffer)
    {
      throw std::runtime_error("Failed to get back buffer from swap chain.");
    }

    // use the back buffer address to create the render target
    _device->CreateRenderTargetView(pBackBuffer, NULL, &_backbuffer);
    pBackBuffer->Release();

    // set the render target as the back buffer
    _context->OMSetRenderTargets(1, &_backbuffer, NULL);

    // Set the viewport
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<FLOAT>(width);
    viewport.Height = static_cast<FLOAT>(height);

    _context->RSSetViewports(1, &viewport);
  }

  D3D11Context::~D3D11Context()
  {
    _swapchain->SetFullscreenState(FALSE, NULL);
    _context->Release();
    _backbuffer->Release();
    _device->Release();
    _swapchain->Release();

    // TEMP
    pVS->Release();
    pPS->Release();
    pVBuffer->Release();
    pLayout->Release();
  }

  void D3D11Context::Setup() noexcept
  {
    static UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

    ID3DBlob *VS {};
    ID3DBlob *PS {};
    ID3DBlob *errorBlob {};
    if (FAILED(D3DCompile(hlslSource.data(), hlslSource.size(), nullptr, nullptr,
                          D3D_COMPILE_STANDARD_FILE_INCLUDE, "PShader", "ps_4_0", compileFlags, 0, &PS,
                          &errorBlob)))
    {
      if (errorBlob)
      {
        OutputDebugStringA((char *)errorBlob->GetBufferPointer());
        errorBlob->Release();
      }
      assert(false && "Failed to compile vertex shader.");
    }
    if (errorBlob)
    {
      errorBlob->Release();
    }
    if (FAILED(D3DCompile(hlslSource.data(), hlslSource.size(), nullptr, nullptr,
                          D3D_COMPILE_STANDARD_FILE_INCLUDE, "VShader", "vs_4_0", compileFlags, 0, &VS,
                          &errorBlob)))
    {
      if (errorBlob)
      {
        OutputDebugStringA((char *)errorBlob->GetBufferPointer());
        errorBlob->Release();
      }
      assert(false && "Failed to compile pixel shader.");
    }
    if (errorBlob)
    {
      errorBlob->Release();
    }

    // encapsulate both shaders into shader objects
    _device->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &pVS);
    _device->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &pPS);

    // set the shader objects
    _context->VSSetShader(pVS, 0, 0);
    _context->PSSetShader(pPS, 0, 0);

    D3D11_BUFFER_DESC bd {};

    bd.Usage = D3D11_USAGE_DYNAMIC; // write access access by CPU and GPU
    bd.ByteWidth = sizeof(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;    // use as a vertex buffer
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // allow CPU to write in buffer

    _device->CreateBuffer(&bd, NULL, &pVBuffer); // create the buffer
    if (!pVBuffer)
    {
      assert(false);
      return;
    }

    D3D11_MAPPED_SUBRESOURCE ms;
    _context->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms); // map the buffer
    memcpy(ms.pData, vertices, sizeof(vertices));                      // copy the data
    _context->Unmap(pVBuffer, NULL);                                   // unmap the buffer

    D3D11_INPUT_ELEMENT_DESC ied[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    _device->CreateInputLayout(ied, 1, VS->GetBufferPointer(), VS->GetBufferSize(), &pLayout);
    _context->IASetInputLayout(pLayout);
  }

  void D3D11Context::Render(ICamera &camera) noexcept
  {
    FLOAT clearColor[4] = {0.3f, 0.3f, 0.3f, 1.0f};
    _context->ClearRenderTargetView(_backbuffer, clearColor);

    UINT stride = sizeof(float) * 3;                                 // size of one vertex
    UINT offset = 0;                                                 // start from the beginning of the buffer
    _context->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset); // set the vertex buffer
    _context->IASetPrimitiveTopology(
      D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // use triangles as the primitive type

    // draw the vertex buffer to the back buffer
    _context->Draw(3, 0);
  }

  void D3D11Context::SetSkybox(const IO::CubeMapImage &skybox) noexcept
  {
  }

  void D3D11Context::Present() noexcept
  {
    _swapchain->Present(0, 0);
  }

  void D3D11Context::Resize(uint32 width, uint32 height) noexcept
  {
    if (_backbuffer)
    {
      _backbuffer->Release();
      _backbuffer = nullptr;
    }
    // resize the swap chain
    _swapchain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    // get the address of the back buffer
    ID3D11Texture2D *pBackBuffer {};
    _swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&pBackBuffer);
    if (!pBackBuffer)
    {
      return;
    }
    // use the back buffer address to create the render target
    _device->CreateRenderTargetView(pBackBuffer, NULL, &_backbuffer);
    pBackBuffer->Release();
    // set the render target as the back buffer
    _context->OMSetRenderTargets(1, &_backbuffer, NULL);
    // Set the viewport
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<FLOAT>(width);
    viewport.Height = static_cast<FLOAT>(height);
    _context->RSSetViewports(1, &viewport);
  }
}
