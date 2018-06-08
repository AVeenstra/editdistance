uint minimum(uint a, uint b, uint c) {
    return min(min(a,b),c);
}

__kernel void calc_diagonals(
    __global char* sequence_a,
    __global char* sequence_b,
    uint height,
    uint offset_a,
    uint offset_b,
    __global uint* diagonals
) {
    size_t id = get_local_id(0);
    char a = sequence_a[get_local_size(0)-id-1];

    uint y = id * 2 + 1;
    for (int i = 0; i < height; ++i, ++y) {
        barrier(CLK_LOCAL_MEM_FENCE);

        diagonals[y] = min(min(diagonals[y - 1], diagonals[y + 1]) + 1, a == sequence_b[id + i] ? diagonals[y] : diagonals[y] + 1);
    }
}

__kernel void fill_row(
    __global char* sequence_a,
    __global char* sequence_b,
    uint height,
    uint offset_a,
    uint offset_b,
    __global uint* diagonals
) {
    size_t id = get_local_id(0);
    size_t width = get_local_size(0);
    char a = sequence_a[id];

    diagonals[id] = offset_a + width - id;

    if (id == 0) {
        diagonals[width] = offset_a;
    }

	uint y = width - id * 2;
    for (int i = 0; i < width - 1; i++, y++) {
        barrier(CLK_GLOBAL_MEM_FENCE);
		if (id<=i) {
            diagonals[y] = min(min(diagonals[y - 1], diagonals[y + 1]) + 1, a == sequence_b[i - id] ? diagonals[y] : diagonals[y] + 1);
		}
    }

    calc_diagonals(sequence_a, sequence_b, height, offset_a, offset_b, diagonals);
}

__kernel void fill_defaults_fill_row(
    __global char* sequence_a,
    __global char* sequence_b,
    uint height,
    uint offset_a,
    uint offset_b,
    __global uint* diagonals
) {
    size_t id = get_local_id(0);
    size_t width = get_local_size(0);
    uint total_size = width * 2 + height;
    for (uint i = width + id + 1; i < total_size; i = i + width) {
        diagonals[i] = i-width;
    }

    fill_row(sequence_a, sequence_b, height, offset_a, offset_b, diagonals);
}

__kernel void fill_defaults_calc_diagonals(
    __global char* sequence_a,
    __global char* sequence_b,
    uint height,
    uint offset_a,
    uint offset_b,
    __global uint* diagonals
) {
    size_t id = get_local_id(0);
    size_t width = get_local_size(0);
    uint total_size = width * 2 + height;
    for (uint i = width * 2 - 1 + id; i <= total_size; i = i + width) {
        diagonals[i] = i - width + offset_b;
    }

    calc_diagonals(sequence_a, sequence_b, height, offset_a, offset_b, diagonals);
}
