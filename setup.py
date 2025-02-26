# SPDX-FileCopyrightText: 2025 Dominik Krupke <krupked@gmail.com>
# SPDX-License-Identifier: MIT

from pathlib import Path

from setuptools import find_packages
from skbuild import setup


def readme():
    """
    :return: Content of README.md
    """

    with Path("README.py.md").open() as file:
        return file.read()


setup(  # https://scikit-build.readthedocs.io/en/latest/usage.html#setup-options
    name="pycft",
    version="0.0.1",
    author="Francesco Cavaliere and Dominik Krupke",
    license="LICENSE",
    description="Python-Bindings for the C++-based Set Cover Algorithm CFT.",
    long_description=readme(),
    long_description_content_type="text/markdown",
    packages=find_packages("src"),  # Include all packages in `./src`.
    package_dir={"": "src"},  # The root for our python package is in `./src`.
    python_requires=">=3.10",  # lowest python version supported.
    install_requires=[  # Python Dependencies
    ],
    cmake_minimum_required_version="3.23",
)
