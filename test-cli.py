import subprocess
from tqdm import tqdm

CLI_PATH = "cmake-build-release/btm"

content = None
with open('example_jhotdraw.txt') as f:
    content = "".join(f.readlines())

total_iterations = 200
process = subprocess.Popen([CLI_PATH, '--iterations', str(total_iterations)], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
process.stdin.write(content.encode("utf-8"))
process.stdin.close()

doctop = []

with tqdm(total=total_iterations) as pbar:
    while True:
        from_program = process.stdout.readline().decode("utf-8")
        if not from_program:
            break
        if from_program.startswith("#progress "):
            pbar.n = int(from_program[len("#progress "):].split(" ")[0])
            pbar.update(0)
        elif from_program.startswith("#doctop "):
            data = [float(x) for x in from_program[len("#doctop "):].strip().split(",")]
            doctop.append(data)
        else:
            print("[btm] " + from_program, end="")
print("doctop received: ", len(doctop))
print("Process exited with code {}".format(process.poll()))
