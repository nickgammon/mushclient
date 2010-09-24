// blending.h
//  Defines for: CMiniWindow::BlendImage
//    and        CMUSHclientDoc::BlendPixel

// colour stuff

#define CLAMP(x) (((x) < 0) ? 0 : (x > 255) ? 255 : (x))


// see: http://www.nathanm.com/photoshop-blending-math/
// and: http://www.pegtop.net/delphi/articles/blendmodes/                   

#define uint8 unsigned char
#define sqr(x) ((x)*(x))    // square of x

// A = blend, B = base

#define Blend_Normal(A,B)	((uint8)(A))
#define Blend_Lighten(A,B)	((uint8)((B > A) ? B:A))
#define Blend_Darken(A,B)	((uint8)((B > A) ? A:B))
#define Blend_Multiply(A,B)	((uint8)((A * B) / 255))
#define Blend_Average(A,B)	((uint8)((A + B) / 2))
#define Blend_Add(A,B)	((uint8)((A + B > 255) ? 255:(A + B)))
#define Blend_Subtract(A,B)	((uint8)((A + B < 255) ? 0:(A + B - 255)))
#define Blend_Difference(A,B)	((uint8)(abs(A - B)))
#define Blend_Negation(A,B)	((uint8)(255 - abs(255 - A - B)))
#define Blend_Screen(A,B)	((uint8)(255 - (((255 - A) * (255 - B)) >> 8)))
#define Blend_Exclusion(A,B)	((uint8)(A + B - 2 * A * B / 255))
#define Blend_Overlay(A,B)	((uint8)((B < 128) ? (2 * A * B / 255):(255 - 2 * (255 - A) * (255 - B) / 255)))
#define Blend_SoftLight(A,B)	((uint8) ((A * B) >> 8) +  ((B * (255 - (((255 - B) * (255-A)) >> 8) - ((A * B) >> 8) )) >> 8) )           // c = ((A * B) >> 8)
#define Blend_HardLight(A,B)	(Blend_Overlay(B,A))
#define Blend_ColorDodge(A,B)	((uint8)((A == 255) ? A:((((unsigned long) B) << 8 ) / (255 - A) > 255) ? 255:((((unsigned long) B) << 8 ) / (255 - A))))
#define Blend_ColorBurn(A,B)	((uint8)((A == 0) ? 0:((255 - (( ((unsigned long)(255 - B)) << 8 ) / A)) < 0) ? 0:(255 - (( ((unsigned long)(255 - B)) << 8 ) / A))))
#define Blend_LinearDodge(A,B)	(Blend_Add(A,B))
#define Blend_LinearBurn(A,B)	(Blend_Subtract(A,B))
#define Blend_LinearLight(A,B)	((uint8)(A < 128) ? Blend_LinearBurn((2 * A),B):Blend_LinearDodge((2 * (A - 128)),B))
#define Blend_VividLight(A,B)	((uint8)(A < 128) ? Blend_ColorBurn((2 * A),B):Blend_ColorDodge((2 * (A - 128)),B))
#define Blend_PinLight(A,B)	((uint8)(A < 128) ? Blend_Darken((2 * A),B):Blend_Lighten((2 *(A - 128)),B))
#define Blend_HardMix(A,B)	((uint8)(A < 255 - B) ? 0:255)
#define Blend_Reflect(A,B)	((uint8)((B == 255) ? B:((A * A / (255 - B) > 255) ? 255:(A * A / (255 - B)))))
#define Blend_Glow(A,B)	(Blend_Reflect(B,A))
#define Blend_Phoenix(A,B)	((uint8)(min(A,B) - max(A,B) + 255))
#define Blend_Opacity(A,B,F,O)	((uint8)(O * F(A,B) + (1 - O) * B))

#define Simple_Opacity(B,X,O)  ((uint8)(O * X + (1 - O) * B))

#define Blend_InverseColorDodge(A,B)	Blend_ColorDodge (B, A)
#define Blend_InverseColorBurn(A,B)	Blend_ColorBurn (B, A)
#define Blend_Freeze(A,B)	((uint8)((A == 0) ? A:((255 - sqr(255 - B)/ A < 0) ? 0:(255 - sqr(255 - B) / A))))
#define Blend_Heat(A,B)	 Blend_Freeze (B, A)
#define Blend_Stamp(A,B)	((uint8)((B + 2*A - 256 < 0) ? 0 : (B + 2*A - 256 > 255) ? 255 : (B + 2*A - 256)    ))
#define Blend_Interpolate(A,B)	((uint8)((cos_table [A] + cos_table [B]> 255) ? 255 : (cos_table [A] + cos_table [B])  ))

#define Blend_Xor(A,B)	((uint8) A ^ B )
#define Blend_And(A,B)	((uint8) A & B )
#define Blend_Or(A,B)	((uint8) A | B )
#define Blend_A(A,B)	((uint8) (A))
#define Blend_B(A,B)	((uint8) (B))
