glslc -fshader-stage=vertex vert.glsl -o vert.spv 
glslc -fshader-stage=fragment frag.glsl -o frag.spv
glslc -fshader-stage=vertex pass.glsl -o pass.spv
glslc -fshader-stage=fragment post.glsl -o post.spv
pause