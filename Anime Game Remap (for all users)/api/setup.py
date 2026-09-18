from glob import glob
import os
import shutil
import subprocess
import sys
import numpy
from setuptools import setup, Extension, find_packages
from Cython.Build import cythonize
from pybind11.setup_helpers import Pybind11Extension, build_ext, ParallelCompile

# Enable parallel compilation for pybind11
ParallelCompile("NPY_NUM_BUILD_JOBS", needs_recompile=lambda obj, src: True, default=4).install()


# Safely inject ccache and fix the C++ runtime linking issue
class SafeBuildExt(build_ext):
    def build_extensions(self):
        # 1. Force the linker to use g++ instead of gcc for C++ extensions
        if hasattr(self.compiler, 'linker_so'):
            self.compiler.linker_so = ["g++"] + self.compiler.linker_so[1:]

        # 2. Prepend ccache if it is available on your Linux/WSL system
        if shutil.which("ccache"):
            for key in ['compiler', 'compiler_so', 'compiler_cxx', 'linker_so']:
                if hasattr(self.compiler, key):
                    current_exe = getattr(self.compiler, key)
                    if current_exe and "ccache" not in current_exe:
                        setattr(self.compiler, key, ["ccache"] + current_exe)
                        
        super().build_extensions()
        self._generate_pyi_stubs()

    def _generate_pyi_stubs(self):
        try:
            import pybind11_stubgen
        except ModuleNotFoundError:
            self.warn("pybind11-stubgen is not installed; skipping .pyi generation")
            return

        for ext in self.extensions:
            if not isinstance(ext, Pybind11Extension):
                continue

            module_name = ext.name
            # Write stubs under the package source root so they persist
            # (pybind11_stubgen will create the package subpath inside this dir)
            source_root = os.path.abspath(os.path.join(os.path.dirname(__file__), "src"))
            output_dir = source_root
            os.makedirs(output_dir, exist_ok=True)

            cmd = [
                sys.executable,
                "-m",
                "pybind11_stubgen",
                "--output-dir",
                output_dir,
                module_name,
            ]

            env = os.environ.copy()

            if hasattr(self, 'build_lib') and self.build_lib:
                build_search_dir = os.path.abspath(self.build_lib)
                env["PYTHONPATH"] = (
                    build_search_dir
                    + os.pathsep
                    + source_root
                    + os.pathsep
                    + env.get("PYTHONPATH", "")
                )
            else:
                env["PYTHONPATH"] = (
                    source_root
                    + os.pathsep
                    + env.get("PYTHONPATH", "")
                )

            self.announce(
                f"Generating .pyi stubs for {module_name} in {output_dir}",
                level=3
            )

            try:
                subprocess.run(cmd, check=True, env=env)
            except subprocess.CalledProcessError as exc:
                raise RuntimeError(
                    f"pybind11_stubgen failed for {module_name}: {exc}"
                ) from exc


pybindExtensions = [
    Pybind11Extension(
        "FixRaidenBoss2.core",
        sorted(glob("src/FixRaidenBoss2/cpp/src/**/*.cpp", recursive=True)),
        cxx_std=17,
    )
]

cyExtensions = cythonize([
    Extension(
        name="FixRaidenBoss2.CyDictTools",
        sources=["src/FixRaidenBoss2/cy/src/tools/DictTools.pyx"],
        include_dirs=[numpy.get_include()],
    ),

    Extension(
        name="FixRaidenBoss2.CyListTools",
        sources=["src/FixRaidenBoss2/cy/src/tools/ListTools.pyx"]
    ),
])

extensions = pybindExtensions + cyExtensions

setup(
    name="FixRaidenBoss2",
    packages=find_packages("src"),
    package_dir={"": "src"},
    ext_modules=extensions,
    cmdclass={"build_ext": SafeBuildExt},
    zip_safe=False,
)