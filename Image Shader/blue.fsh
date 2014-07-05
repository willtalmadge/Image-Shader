varying highp vec2 textureCoordinate;
varying highp float colorMultiplier;

uniform sampler2D inputImageTexture;

void main()
{
    mediump vec4 textureColor = texture2D(inputImageTexture, textureCoordinate);
    textureColor.rgb = colorMultiplier*textureColor.rgb;
    gl_FragColor = textureColor;
}