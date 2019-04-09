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

Add a fog factor $\lambda$, Blend fog with the scene

1. Linear Fog: $\lambda=(f-d)/(f-n)$
2. Exponential Fog: $\lambda=(\frac{1}{e})^{d}$
3. Exponential Fog 2: $\lambda=(\frac{1}{e})^{d^2}$

Where $f$ and $n$ are the far plane and the near plane, $d$ is the distance from camera, $e$ is the base of natural logarithm.

And finally the blend function is: $c=\lambda c+(1-\lambda)i$, where $c$ is texture color and $i$ is fog intensity.

## Tut 24 Clip Plane

Addtion to frustum clipping, clip on arbitrary plane.

DirectX 11 sematics:

- **SV_CullDistance\[n\]**: Primitives with this value less than 0 for all vertices will be discarded.
- **SV_ClipDistance\[n\]**: Pixels with this value less than 0 in a primitive will not be rasterized.

Plane equation $dot(p-p_0, n)=0$, to homogeneous space $dot(\begin{bmatrix}p&1\end{bmatrix},\begin{bmatrix}n,-dot(n,p_0)\end{bmatrix})=0$

When a point is on the side that n points, clip value $v=dot(\begin{bmatrix}p&1\end{bmatrix},\begin{bmatrix}n,-dot(n,p_0)\end{bmatrix})\geq 0$. So $v$ will be calculated for each vertex in the vertex shader.

## Tut 27 Reflection

Plane mirror

1. Mirror plane and camera position -> image camera position
2. View from image camera, render the scene to an texture
3. Render the whole scene with out the mirror to RTV as usual
4. Render the mirror, for each rasterized pixel, find its projection space coordinate in image camera view, sample from the texture in 2. to get real color image color in the mirror. Since light path is reversible, every pixel of the mirror that can be seen in the real camera can be seen in the image camera, so all of them can be sampled.

## Tut 28 Fade(Not implemented here)

- Fade render:
  1. Render the scene to a texture
  2. Map the texture to back buffer, multipled by a fade coef
- Normal render:
  1. Render the scene to back buffer

Start with face coef valued 0, do a fade render. Then accumulate fade coef over time. When done(fade coef goes 1), return to normal render.

## Tut 29 Water

- Fresnel water surface:
  - Reflection texture
  - Refraction texture
- Bump map to simulate waves
