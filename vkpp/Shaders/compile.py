import os
import sys
import subprocess
import shutil

working_dir_before_execute = os.getcwd()
script_root = os.path.dirname(os.path.realpath(__file__))
working_dir_stack = list()

def chdir(path: str):
    os.chdir(path)
    print("Working dir:", os.getcwd())

def pushd(dir):
    working_dir_stack.append(os.getcwd())
    os.chdir(dir)
    print("Working dir:", os.getcwd())

def popd():
    os.chdir(working_dir_stack.pop())
    print("Working dir:", os.getcwd())

def run(command : str, env = os.environ):
    print("Execute:", command)
    subprocess.run(command, shell=True, env=env)

def remove_dir(dir : str):
    if os.path.isdir(str):
        shutil.rmtree(dir)

glslang = os.environ["VULKAN_SDK"] + "/Bin/glslang"
default_options = f"-V"
tasks = [
    {
        "source" : "Gui/BlitToSwapchain.vert",
        "output" : "Compiled/Gui/BlitToSwapchain.vert.inc",
        "options" : "-x",
    },
    {
        "source" : "Gui/BlitToSwapchain.frag",
        "output" : "Compiled/Gui/BlitToSwapchain.frag.inc",
        "options" : "-x",
    },
]

def compile_shaders():
    for task in tasks:
        output_dir = os.path.dirname(task['output'])
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
        run(f"{glslang} {task['source']} -o {task['output']} {default_options} {task['options']}")

def main() -> int:
    try:
        chdir(script_root)
        compile_shaders()
        chdir(working_dir_before_execute)
        return 0
    except Exception as e:
        print(e)
        chdir(working_dir_before_execute)
        return -1

if __name__ == '__main__':
    sys.exit(main())
