del *.spv
glslc -fshader-stage=vertex vert.glsl -o vert.spv 
glslc -fshader-stage=fragment conventional_frag.glsl -o conventional_frag.spv
glslc -fshader-stage=fragment pbr_frag.glsl -o pbr_frag.spv
glslc -fshader-stage=vertex pass.glsl -o pass.spv
glslc -fshader-stage=fragment post.glsl -o post.spv
pause
