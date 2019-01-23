common [[
    struct PSInput
    {
        float4 position : SV_POSITION;
        float4 color : COLOR;
    };
]]

vertex_shader [[
    PSInput main(float4 position : POSITION, float4 color : COLOR)
    {
        PSInput result;

        result.position = position;
        result.color = color;

        return result;
    }
]]

fragment_shader [[
    float4 main(PSInput input) : SV_TARGET
    {
        return input.color;
    }
]]

compute_shader [[
    [numthreads(1, 1, 1)]
    void CSMain( uint3 DTid : SV_DispatchThreadID )
    {
    }
]]
