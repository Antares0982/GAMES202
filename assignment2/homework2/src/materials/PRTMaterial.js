class PRTMaterial extends Material {
    constructor(precomputeL_local, vertexShader, fragmentShader) {
        let precomputeL_mat = getMat3ValueFromRGB(precomputeL_local[guiParams.envmapId]);

        super({
            'uPrecomputeLR': { type: 'matrix3fv', value: precomputeL_mat[0] },
            'uPrecomputeLG': { type: 'matrix3fv', value: precomputeL_mat[1] },
            'uPrecomputeLB': { type: 'matrix3fv', value: precomputeL_mat[2] },
        }, ['aPrecomputeLT'], vertexShader, fragmentShader, null);
    }
}

async function buildPRTMaterial(precomputeL_local, vertexPath, fragmentPath) {
    let vertexShader = await getShaderString(vertexPath);
    let fragmentShader = await getShaderString(fragmentPath);

    return new PRTMaterial(precomputeL_local, vertexShader, fragmentShader);
}