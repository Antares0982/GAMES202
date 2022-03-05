function getRotationPrecomputeL(precompute_L, rotationMatrix) {
	let rotation = mat4Matrix2mathMatrix(rotationMatrix);
	let rotate3 = computeSquareMatrix_3by3(rotation);
	let rotate5 = computeSquareMatrix_5by5(rotation);

	let ans = [];
	for (let i = 0; i < 3; i++) {
		let precompute_L_row = math.clone(precompute_L[i]);
		ans[i] = new Float32Array(9);

		let d1 = precompute_L_row[1];
		let d2 = precompute_L_row[2];
		let d3 = precompute_L_row[3];
		let d4 = precompute_L_row[4];
		let d5 = precompute_L_row[5];
		let d6 = precompute_L_row[6];
		let d7 = precompute_L_row[7];
		let d8 = precompute_L_row[8];

		let rotated3 = math.multiply(math.matrix(rotate3), math.matrix([[d1], [d2], [d3]]));
		let rotated5 = math.multiply(math.matrix(rotate5), math.matrix([[d4], [d5], [d6], [d7], [d8]]));
		ans[i][0] = precompute_L_row[0];
		ans[i][1] = rotated3._data[0];
		ans[i][2] = rotated3._data[1];
		ans[i][3] = rotated3._data[2];
		ans[i][4] = rotated5._data[0];
		ans[i][5] = rotated5._data[1];
		ans[i][6] = rotated5._data[2];
		ans[i][7] = rotated5._data[3];
		ans[i][8] = rotated5._data[4];
	}
	return ans;
}

function computeSquareMatrix_3by3(rotationMatrix) { // 计算方阵SA(-1) 3*3 
	// 1、pick ni - {ni}
	let n1 = [1, 0, 0, 0]; let n2 = [0, 0, 1, 0]; let n3 = [0, 1, 0, 0];

	// 2、{P(ni)} - A  A_inverse
	let c1 = SHEval(n1[0], n1[1], n1[2], 3);
	let c2 = SHEval(n2[0], n2[1], n2[2], 3);
	let c3 = SHEval(n3[0], n3[1], n3[2], 3);
	let p = math.matrix([[c1[1], c2[1], c3[1]], [c1[2], c2[2], c3[2]], [c1[3], c2[3], c3[3]]]);
	let Ainv = math.inv(p);

	// 3、用 R 旋转 ni - {R(ni)}
	let nn1 = math.multiply(rotationMatrix, n1);
	let nn2 = math.multiply(rotationMatrix, n2);
	let nn3 = math.multiply(rotationMatrix, n3);

	// 4、R(ni) SH投影 - S
	c1 = SHEval(nn1[0], nn1[1], nn1[2], 3);
	c2 = SHEval(nn2[0], nn2[1], nn2[2], 3);
	c3 = SHEval(nn3[0], nn3[1], nn3[2], 3);

	let Rni = math.matrix([[c1[1], c2[1], c3[1]], [c1[2], c2[2], c3[2]], [c1[3], c2[3], c3[3]]]);

	// 5、S*A_inverse
	return math.transpose(math.multiply(Rni, Ainv));
}

function computeSquareMatrix_5by5(rotationMatrix) { // 计算方阵SA(-1) 5*5

	// 1、pick ni - {ni}
	let k = 1 / math.sqrt(2);
	let n1 = [1, 0, 0, 0]; let n2 = [0, 0, 1, 0]; let n3 = [k, k, 0, 0];
	let n4 = [k, 0, k, 0]; let n5 = [0, k, k, 0];

	// 2、{P(ni)} - A  A_inverse
	let c1 = SHEval(n1[0], n1[1], n1[2], 5);
	let c2 = SHEval(n2[0], n2[1], n2[2], 5);
	let c3 = SHEval(n3[0], n3[1], n3[2], 5);
	let c4 = SHEval(n4[0], n4[1], n4[2], 5);
	let c5 = SHEval(n5[0], n5[1], n5[2], 5);

	let p = math.matrix([
		[c1[4], c2[4], c3[4], c4[4], c5[4]],
		[c1[5], c2[5], c3[5], c4[5], c5[5]],
		[c1[6], c2[6], c3[6], c4[6], c5[6]],
		[c1[7], c2[7], c3[7], c4[7], c5[7]],
		[c1[8], c2[8], c3[8], c4[8], c5[8]],
	]);
	let Ainv = math.inv(p);

	// 3、用 R 旋转 ni - {R(ni)}
	n1 = math.multiply(rotationMatrix, n1);
	n2 = math.multiply(rotationMatrix, n2);
	n3 = math.multiply(rotationMatrix, n3);
	n4 = math.multiply(rotationMatrix, n4);
	n5 = math.multiply(rotationMatrix, n5);

	// 4、R(ni) SH投影 - S
	c1 = SHEval(n1[0], n1[1], n1[2], 5);
	c2 = SHEval(n2[0], n2[1], n2[2], 5);
	c3 = SHEval(n3[0], n3[1], n3[2], 5);
	c4 = SHEval(n4[0], n4[1], n4[2], 5);
	c5 = SHEval(n5[0], n5[1], n5[2], 5);

	let Rni = math.matrix([
		[c1[4], c2[4], c3[4], c4[4], c5[4]],
		[c1[5], c2[5], c3[5], c4[5], c5[5]],
		[c1[6], c2[6], c3[6], c4[6], c5[6]],
		[c1[7], c2[7], c3[7], c4[7], c5[7]],
		[c1[8], c2[8], c3[8], c4[8], c5[8]],
	]);

	// 5、S*A_inverse
	return math.transpose(math.multiply(Rni, Ainv));
}

function mat4Matrix2mathMatrix(rotationMatrix) {

	let mathMatrix = [];
	for (let i = 0; i < 4; i++) {
		let r = [];
		for (let j = 0; j < 4; j++) {
			r.push(Number(rotationMatrix[i * 4 + j]));
		}
		mathMatrix.push(r);
	}
	return mathMatrix;
	//return math.matrix(mathMatrix)

}

function getMat3ValueFromRGB(precomputeL) {

	let colorMat3 = [];
	for (let i = 0; i < 3; i++) {
		colorMat3[i] = mat3.fromValues(precomputeL[0][i], precomputeL[1][i], precomputeL[2][i],
			precomputeL[3][i], precomputeL[4][i], precomputeL[5][i],
			precomputeL[6][i], precomputeL[7][i], precomputeL[8][i]);
	}
	return colorMat3;
}

function getMat3ValueFromRGB2(precomputeL_loc) {
	let colorMat3 = [];
	for (let i = 0; i < 3; i++) {
		colorMat3[i] = new Float32Array(9);
		for (let j = 0; j < 9; j++) {
			colorMat3[i][j] = precomputeL_loc[j][i];
		}
	}
	return colorMat3;
}
