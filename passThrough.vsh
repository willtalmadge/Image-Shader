attribute vec4 position;
attribute vec4 inputTextureCoordinate;
attribute float inputColorMultiplier;

varying vec2 textureCoordinate;
varying float colorMultiplier;
uniform mat4 orthoMatrix;

void main()
{
    gl_Position = orthoMatrix*position;
    textureCoordinate = inputTextureCoordinate.xy;
    colorMultiplier = inputColorMultiplier;
}