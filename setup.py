from pathlib import Path

from setuptools import find_packages
from skbuild import setup


def readme():
    """
    :return: Content of README.md
    """

    with Path("README.md").open() as file:
        return file.read()


setup(  # https://scikit-build.readthedocs.io/en/latest/usage.html#setup-options
    name="pyaccft",
    version="0.0.1",
    author="TODO",
    license="LICENSE",
    description="Pybinding for accft",
    long_description=readme(),
    long_description_content_type="text/markdown",
    packages=find_packages("src"),  # Include all packages in `./src`.
    package_dir={"": "src"},  # The root for our python package is in `./src`.
    python_requires=">=3.10",  # lowest python version supported.
    install_requires=[  # Python Dependencies
    ],
    cmake_minimum_required_version="3.23",
)
