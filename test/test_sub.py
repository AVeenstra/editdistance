import sys, os, math

root = "/home/veenstraaj/CLionProjects/edit_distance_code/cmake-build-release/sequences"
seq = {
    "a": "Homo_sapiens.GRCh38.dna.chromosome.1.filtered.fa",
    "b": "Homo_sapiens.GRCh38.dna.chromosome.2.filtered.fa"
}
base = "seq{}{}"

for size in list(range(int(sys.argv[1]), int(sys.argv[2]), int(sys.argv[1]))) + list(int(x) for x in sys.argv[3:]):
    ssize = int(math.sqrt(size))
    for c in ['a', 'b']:
        target = os.path.join(root, base.format(c, size))
        if not os.path.isfile(target):
            file = open(os.path.join(root, seq[c]), "rb")
            output = open(target, "wb")
            output.write(file.read(ssize))
            output.close()
            file.close()
