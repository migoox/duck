#pragma once

#include "DirectXMath.h"
#include "camera.h"
#include "clock.h"
#include "d3d11.h"
#include "diInstance.h"
#include "dxDevice.h"
#include "keyboard.h"
#include "mouse.h"
#include "windowApplication.h"

namespace mini
{
class DxApplication : public mini::WindowApplication
{
  public:
    explicit DxApplication(HINSTANCE hInstance, int wndWidth = Window::m_defaultWindowWidth,
                           int wndHeight = Window::m_defaultWindowHeight, std::wstring wndTitle = L"DirectX Window");

  protected:
    int MainLoop() override;

    const Clock& getClock() const
    {
        return m_clock;
    }
    virtual void Render();
    virtual void Update(const Clock& c)
    {
    }

    void UpdateBuffer(const dx_ptr<ID3D11Buffer>& buffer, const void* data, size_t count);

    template <typename T> void UpdateBuffer(const dx_ptr<ID3D11Buffer>& buffer, const T& data)
    {
        UpdateBuffer(buffer, &data, sizeof(T));
    }

    template <typename T> void UpdateBuffer(const dx_ptr<ID3D11Buffer>& buffer, const std::vector<T>& data)
    {
        UpdateBuffer(buffer, data.data(), data.size() * sizeof(T));
    }

    // Resets pipeline back to rendering into program window
    void ResetRenderTarget();

    DxDevice m_device;

    DiInstance m_inputDevice;
    Mouse m_mouse;
    Keyboard m_keyboard;

  protected:
    mini::dx_ptr<ID3D11DepthStencilView> m_depthBuffer;

  private:
    mini::dx_ptr<ID3D11RenderTargetView> m_backBuffer;
    Viewport m_viewport;
    Clock m_clock;
};
} // namespace mini