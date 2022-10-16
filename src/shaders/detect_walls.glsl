// NOTE: This does not work at all. The algorithm was adapted from https://arxiv.org/abs/1509.00026v1 however the implementation
// might be off which causes the detection to be wrong.
#version 460 core
// Work groups
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
// In: Real field texture
layout(rgba32f, binding = 0) restrict readonly uniform image2D inRealFieldTexture;
// In: Imaginary field texture
layout(rgba32f, binding = 1) restrict readonly uniform image2D inImagFieldTexture;
// Out: String texture
layout(r32f, binding = 2) restrict writeonly uniform image2D outStringTexture;


// Returns of the handedness of a real crossing as +-1.
int calculateCrossingHandedness(
    float realCurrent, float imagCurrent, float realNext, float imagNext
) {
        float result = realNext * imagCurrent - realCurrent * imagNext;
        return int(sign(result));
}

// Returns 1 if negative real axis is crossed otherwise 0.
int checkNegativeRealCrossing(
    float realCurrent, float imagCurrent, float realNext, float imagNext
) {
        float result = (imagCurrent - imagNext) * (realNext * imagCurrent - realCurrent * imagNext);
        return int(result < 0);
}

// Returns `1` if the link crosses the real axis, otherwise returns `0`.
int calculateRealCrossing(float imagCurrent, float imagNext) {
    int result = int((imagCurrent * imagNext) < 0);
    return result;
}

// Detects whether a string pierces through the given plaquette, which is a tetragon of points.
int checkPlaquette(
    float realTopLeft, float imagTopLeft,
    float realTopRight, float imagTopRight,
    float realBottomRight, float imagBottomRight,
    float realBottomLeft, float imagBottomLeft
) {
        int result = 0;

        // Check top left to top right link for crossing handedness
        result += calculateRealCrossing(imagTopLeft, imagTopRight)
            * calculateCrossingHandedness(realTopLeft, imagTopLeft, realTopRight, imagTopRight);
        // Check top right to bottom right link for crossing handedness
        result += calculateRealCrossing(imagTopRight, imagBottomRight)
            * calculateCrossingHandedness(realTopRight, imagTopRight, realBottomRight, imagBottomRight);
        // Check bottom right to bottom left link for crossing handedness
        result += calculateRealCrossing(imagBottomRight, imagBottomLeft)
            * calculateCrossingHandedness(realBottomRight, imagBottomRight, realBottomLeft, imagBottomLeft);
        // Check bottom left to top left link for crossing handedness
        result += calculateRealCrossing(imagBottomLeft, imagTopLeft)
            * calculateCrossingHandedness(realBottomLeft, imagBottomLeft, realTopLeft, imagTopLeft);

        return result;
}

// Detects whether a string pierces through the given plaquette, which is a tetragon of points.
int checkPlaquetteWall(
    float realTopLeft, float imagTopLeft,
    float realTopRight, float imagTopRight,
    float realBottomRight, float imagBottomRight,
    float realBottomLeft, float imagBottomLeft
) {
        int result = 0;

        // Check top left to top right link for crossing handedness
        result += calculateRealCrossing(imagTopLeft, imagTopRight)
            * checkNegativeRealCrossing(realTopLeft, imagTopLeft, realTopRight, imagTopRight);
        // Check top right to bottom right link for crossing handedness
        result += calculateRealCrossing(imagTopRight, imagBottomRight)
            * checkNegativeRealCrossing(realTopRight, imagTopRight, realBottomRight, imagBottomRight);
        // Check bottom right to bottom left link for crossing handedness
        result += calculateRealCrossing(imagBottomRight, imagBottomLeft)
            * checkNegativeRealCrossing(realBottomRight, imagBottomRight, realBottomLeft, imagBottomLeft);
        // Check bottom left to top left link for crossing handedness
        result += calculateRealCrossing(imagBottomLeft, imagTopLeft)
            * checkNegativeRealCrossing(realBottomLeft, imagBottomLeft, realTopLeft, imagTopLeft);

        return result;
}

void main()
{
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(inRealFieldTexture);

    // Positions: Horizontal
    ivec2 centreLeftPos = ivec2(mod(pos.x - 1, size.x), pos.y);
    ivec2 centreRightPos = ivec2(mod(pos.x + 1, size.x), pos.y);
    // Positions: Vertical
    ivec2 centreDownPos = ivec2(pos.x, mod(pos.y - 1, size.y));
    ivec2 centreUpPos = ivec2(pos.x, mod(pos.y + 1, size.y));
    // Positions: Diagonals
    ivec2 bottomLeftPos = ivec2(mod(pos.x - 1, size.x), mod(pos.y - 1, size.y));
    ivec2 bottomRightPos = ivec2(mod(pos.x + 1, size.x), mod(pos.y - 1, size.y));
    ivec2 topLeftPos = ivec2(mod(pos.x - 1, size.x), mod(pos.y + 1, size.y));
    ivec2 topRightPos = ivec2(mod(pos.x + 1, size.x), mod(pos.y + 1, size.y));

    // Current cell
    float realCurrent = imageLoad(inRealFieldTexture, pos).r;
    float imagCurrent = imageLoad(inImagFieldTexture, pos).r;
    // Horizontal
    float realCentreLeft = imageLoad(inRealFieldTexture, centreLeftPos).r;
    float imagCentreLeft = imageLoad(inImagFieldTexture, centreLeftPos).r;
    float realCentreRight = imageLoad(inRealFieldTexture, centreRightPos).r;
    float imagCentreRight = imageLoad(inImagFieldTexture, centreRightPos).r;
    // Vertical
    float realCentreDown = imageLoad(inRealFieldTexture, centreDownPos).r;
    float imagCentreDown = imageLoad(inImagFieldTexture, centreDownPos).r;
    float realCentreUp = imageLoad(inRealFieldTexture, centreUpPos).r;
    float imagCentreUp = imageLoad(inImagFieldTexture, centreUpPos).r;
    // Diagonals
    float realBottomLeft = imageLoad(inRealFieldTexture, bottomLeftPos).r;
    float imagBottomLeft = imageLoad(inImagFieldTexture, bottomLeftPos).r;
    float realBottomRight = imageLoad(inRealFieldTexture, bottomRightPos).r;
    float imagBottomRight = imageLoad(inImagFieldTexture, bottomRightPos).r;
    float realTopLeft = imageLoad(inRealFieldTexture, topLeftPos).r;
    float imagTopLeft = imageLoad(inImagFieldTexture, topLeftPos).r;
    float realTopRight = imageLoad(inRealFieldTexture, topRightPos).r;
    float imagTopRight = imageLoad(inImagFieldTexture, topRightPos).r;

    int stringCount = 0;

    // Top left plaquette
    stringCount += checkPlaquette(
        realTopLeft, imagTopLeft,
        realCentreUp, imagCentreUp,
        realCurrent, imagCurrent,
        realCentreLeft, imagCentreLeft
    );
    // Top right plaquette
    stringCount += checkPlaquette(
        realCentreUp, imagCentreUp,
        realTopRight, imagTopRight,
        realCentreRight, imagCentreRight,
        realCurrent, imagCurrent
    );
    // Bottom right plaquette
    stringCount += checkPlaquette(
        realCurrent, imagCurrent,
        realCentreRight, imagCentreRight,
        realBottomRight, imagBottomRight,
        realCentreDown, imagCentreDown
    );
    // Bottom left plaquette
    stringCount += checkPlaquette(
        realCentreLeft, imagCentreLeft,
        realCurrent, imagCurrent,
        realCentreDown, imagCentreDown,
        realBottomLeft, imagBottomLeft
    );

    // Clamp result to between -1 and 1
    bool stringPierced = abs(stringCount) > 0;

    int wallCount = 0;

    // Top left plaquette
    wallCount += checkPlaquetteWall(
        realTopLeft, imagTopLeft,
        realCentreUp, imagCentreUp,
        realCurrent, imagCurrent,
        realCentreLeft, imagCentreLeft
    );
    // Top right plaquette
    wallCount += checkPlaquetteWall(
        realCentreUp, imagCentreUp,
        realTopRight, imagTopRight,
        realCentreRight, imagCentreRight,
        realCurrent, imagCurrent
    );
    // Bottom right plaquette
    wallCount += checkPlaquetteWall(
        realCurrent, imagCurrent,
        realCentreRight, imagCentreRight,
        realBottomRight, imagBottomRight,
        realCentreDown, imagCentreDown
    );
    // Bottom left plaquette
    wallCount += checkPlaquetteWall(
        realCentreLeft, imagCentreLeft,
        realCurrent, imagCurrent,
        realCentreDown, imagCentreDown,
        realBottomLeft, imagBottomLeft
    );

    // If string has pierced
    bool stringPiercedAndWall = stringPierced && (mod(wallCount, 2) == 1);
    bool stringNotPiercedAndWall = !stringPierced && (mod(wallCount, 2) == 0);
    bool hasWall = stringPiercedAndWall || stringNotPiercedAndWall;

    float result = mix(0.0f, 1.0f, hasWall);

    // Store phase
    imageStore(outStringTexture, pos, vec4(result, 0.0f, 0.0f, 0.0f));
}