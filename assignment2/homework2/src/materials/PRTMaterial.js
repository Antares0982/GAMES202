class PRTMaterial extends Material {
    constructor(precomputeL_local, vertexShader, fragmentShader) {
        //console.log(precomputeL_local[guiParams.envmapId]);
        // let precomputeL_mat = getMat3ValueFromRGB(precomputeL_local[guiParams.envmapId]);
        // console.log(precomputeL_mat);

        super({
            'uPrecomputeLR': { type: 'updatedInRealTime', value: null },
            'uPrecomputeLG': { type: 'updatedInRealTime', value: null },
            'uPrecomputeLB': { type: 'updatedInRealTime', value: null },
            //'uSampler': { type: 'texture', value: color },
        }, ['aPrecomputeLT'], vertexShader, fragmentShader, null);
        // super({
        //     'uPrecomputeLR': { type: 'matrix3fv', value: precomputeL_mat[0] },
        //     'uPrecomputeLG': { type: 'matrix3fv', value: precomputeL_mat[1] },
        //     'uPrecomputeLB': { type: 'matrix3fv', value: precomputeL_mat[2] },
        // }, ['aPrecomputeLT'], vertexShader, fragmentShader, null);
    }
}

async function buildPRTMaterial(precomputeL_local, vertexPath, fragmentPath) {
    let vertexShader = await getShaderString(vertexPath);
    let fragmentShader = await getShaderString(fragmentPath);

    return new PRTMaterial(precomputeL_local, vertexShader, fragmentShader);
}