# DXTutorial

## Tut 1, 2, 3

Win32 Framework with DirectX11

## Tut 4 Basic pipeline

1. IAS(VB, IB)
2. VSS(Obj->World->View->Proj)
3. RS(Cull, To NDC, Viewport mapping)
4. PSS(NDC->RenderTarget)
5. OMS(Depth-Stencil test and write, Blend(RGB+A))

## Tut 5 Texture

- Host:SRV for texture in PS, Sampler in PS
- Device:SamplerState, Texture

## Tut 6 Diffuse Light

$$I=texture(uv)\times kd\times dot(n, -l)$$

## Tut 7, 8 Model Reading

Wavefront OBJ file

File Right-handed -> D3D Left-handed:

- Reverse index order in a triangle
- position: reverse Z
- normal: reverse Z
- uv: if texture file is stored in OpenGL way, fine; if in D3D way, y = 1.0 - y

## Tut 9 Ambient Light

$$I=texture(uv)\times (ka+kd\times dot(n, -l))$$

## Tut 10 Specular Light

$$I=texture(uv)\times (ka+kd\times dot(n, -l))+ks\times dot(view-l,n)^{kn}$$

## Tut 11, 12 2D Rendering, Font

- Text->Texture Image
- Runtime VB, IB creation
- Orthogonal projection
- Blend, Pixels not covering text remain untouched, otherwise overwrite

## Tut 13 Direct Input

- DirectInput handle->Input device from handle
- Polling over all devices created, get device status and deal with inputs

## Tut 14 Direct Sound

- DirectSound handle
- PCM wave format->Primary buffer
- .wav header->PCM wave format->Secondary buffer->Copy raw data to buffer

## Tut 15 System Info

- FPS
- CPU Usage(PDH library)

## Tut 16 Frustum Culling

Host based pre-culling

1. Reproject culling space to world space
2. Add AABB for model
3. AABB culling test, if failed, do not render

## Tut 17~20 Multi-texture, Bump map

Bind different textures to different SRV slots

Bump map:

- Tangent space: 
  - T(tangent):Vector on u direction
  - B(binormal):Vector on v direction
  - N(normal):Vector on surface normal direction
  - $N=T\times B$
- Tangent space to World space:
  - Triangle $v_0$, $v_1$, $v_2$
  - $v_1-v_0=(u_1-u_0)T+(v_1-v_0)B$
  - $v_2-v_0=(u_2-u_0)T+(v_2-v_0)B$
  - $\begin{bmatrix}T&B\end{bmatrix}^T=\begin{bmatrix}u_1-u_0&v_1-v_0\\u_2-u_0&v_2-v_0\end{bmatrix}^{-1}\begin{bmatrix}v_1-v_0&v_2-v_0\end{bmatrix}^T$
  - Hence tangent space basis is normalized T, normalized B and normalized $T\times B$
- Bump map:
  - 3 channel stand for coordinates in tangent space
  - r->T, g->B, b->N(So the whole pic is blue)
  - Map rgb from 0~1 to -1~1
  - $normal=normalize(rT+gB+bN)$

## Tut 22 Render to Texture

1. Bind a texture to render target, render one pass.
2. Bind the rendered texture to shader resource, render another pass to use the texture

## Tut 23 Fog

TODO
