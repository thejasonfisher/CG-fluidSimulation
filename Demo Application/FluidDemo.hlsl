//--------------------------------------------------------------------------------------
// File: FluidDemo.hlsl
//
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

// General Vars
matrix World;
matrix View;
matrix Projection;
float4 vCameraPos;

// Phong Vars
float4 vLightDir[3];
float4 vLightColor[3];
float4 vAmbientColor;
float4 vSphereColor;
float  fSpecPower;

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos	: POSITION;
    float3 Norm : NORMAL;
};

struct PS_INPUT
{
	float4 Pos  : SV_POSITION0;
	float4 WPos : POSITION0;
    float3 Norm : TEXCOORD0;
};

struct VS_INPUT_BATCHED
{
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
	float4 Color : COLOR;
	row_major float4x4 World : WORLD;
};

struct PS_INPUT_BATCHED
{
	float4 Pos		: SV_POSITION0;
	float4 WPos		: POSITION0;
    float3 Norm		: TEXCOORD0;
	float4 Color	: Color;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
	PS_INPUT output = (PS_INPUT)0;
	
	output.WPos = mul( input.Pos, World );
	output.Pos  = mul( output.WPos, View );
	output.Pos  = mul( output.Pos, Projection );

	output.Norm = normalize( mul( input.Norm, (float3x3)World ) );

	return output;
}

//------------------
PS_INPUT_BATCHED VS_BATCHED( VS_INPUT_BATCHED input )
{
	PS_INPUT_BATCHED output = (PS_INPUT_BATCHED)0;
	
	output.WPos = mul( input.Pos, input.World );
	output.Pos  = mul( output.WPos, View );
	output.Pos  = mul( output.Pos, Projection );

	output.Norm = normalize( mul( input.Norm, (float3x3)World ) );
	output.Color = input.Color;

	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader - Phong Shading
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	float4 result = float4( 0.0f, 0.0f, 0.0f, 1.0f );

	//calculate lighting
	float3 V = normalize( vCameraPos - (float3) input.WPos );
	float3 norm = normalize( input.Norm );

	[unroll]
    for(int i=0; i<1; i++)
    {
		float3 lightDir = normalize( vLightDir[i] );

		float3 R = reflect( lightDir, norm );
		 
		float4 vAmbient  = 0.025f * vAmbientColor;
		float4 vDiffuse  = 0.7f * saturate( dot( norm, (float3) -lightDir ) );
		float4 vSpecular = 0.1f * pow( saturate(dot( R,V )), fSpecPower );

		result += vAmbient + ( vDiffuse + vSpecular ) * vLightColor[i];
	}

	float4 final = result * float4( 0.0f, 1.0f, 0.0f, 1.0f );
	final.a = 1.0f;
    return final;
}

//--------------------------------------------------------------------------------------
// Pixel Shader - Phong Shading 
//--------------------------------------------------------------------------------------
float4 PS_BATCHED( PS_INPUT_BATCHED input) : SV_Target
{
	float4 result = float4( 0.0f, 0.0f, 0.0f, 1.0f );

	//calculate lighting
	float3 V = normalize( vCameraPos - (float3) input.WPos );
	float3 norm = normalize( input.Norm );

	[unroll]
    for(int i=0; i<3; i++)
    {
		float3 lightDir = normalize( vLightDir[i] );

		float3 R = reflect( lightDir, norm );
		 
		float4 vAmbient  = 0.025f * vAmbientColor;
		float4 vDiffuse  = 0.4f * saturate( dot( norm, (float3) -lightDir ) );
		float4 vSpecular = 0.6f * pow( saturate(dot( R,V )), fSpecPower );

		result += vAmbient + ( vDiffuse + vSpecular ) * vLightColor[i];
	}

	float4 final = result * input.Color;
	final.a = 1.0f;
    return final;
}

//--------------------------------------------------------------------------------------
// Pixel Shader - Solid Shading 
//--------------------------------------------------------------------------------------
float4 PS_BATCHED_SQUARE( PS_INPUT_BATCHED input) : SV_Target
{
	float4 color = float4( 0.0f, 0.0f, 0.0f, 1.0f );
	color.x = lerp( 0.0f, 1.0f, input.Color.x );

	color.a = 1.0f;
    return color;
}

//--------------------------------------------------------------------------------------
// Pixel Shader - Solid Shading
//--------------------------------------------------------------------------------------
float4 PS_SOLID( PS_INPUT input) : SV_Target
{
	return vSphereColor;
}

//--------------------------------------------------------------------------------------
technique10 RenderPhong
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}

//--------------------------------------------------------------------------------------
technique10 RenderPhongBatched
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS_BATCHED() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_BATCHED() ) );
    }
}

technique10 RenderSolidSquareBatched
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS_BATCHED() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_BATCHED_SQUARE() ) );
    }
}

technique10 RenderSolid
{
	pass P0
	{
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_SOLID() ) );		
	}
}




