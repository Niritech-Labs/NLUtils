
#ifdef VSH
uniform mat4 ProjMtx;
in vec2 Position;
in vec2 TexCoord;
in vec4 Color;
out vec2 Frag_UV;
out vec4 Frag_Color;
void main() {
    Frag_UV = TexCoord;
    Frag_Color = Color;
    gl_Position = ProjMtx * vec4(Position, 0.0, 1.0);
}
#endif

#ifdef FSH
precision mediump float;
uniform sampler2D Texture;
in vec2 Frag_UV;
in vec4 Frag_Color;
out vec4 Out_Color;
void main() {
    Out_Color = Frag_Color * texture(Texture, Frag_UV);
}
#endif