varying highp vec2 textureCoordinate;

uniform sampler2D inputImageTexture;

void main()
{
    highp vec4 textureColor = texture2D(inputImageTexture, textureCoordinate);
    textureColor.a = 1.0;
    textureColor.rgb = textureColor.rgb;
    gl_FragColor = textureColor;
}