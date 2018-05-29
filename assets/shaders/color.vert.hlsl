struct VSInput
{
	float4 position : TEXCOORD0;
	float4 color : TEXCOORD1;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

PSInput main(VSInput input)
{
	PSInput result;

	result.position = input.position;
	result.color = input.color;

	return result;
}
