#!/usr/bin/env python3

from conans import ConanFile, CMake

class NotNullConan(ConanFile):

    # Package Info
    name = "NotNull"
    version = "0.0.1"
    description = " A wrapper type to prevent nullability"
    url = "https://github.com/bitwizeshift/not_null"
    author = "Matthew Rodusek <matthew.rodusek@gmail.com>"
    license = "MIT"

    # Sources
    exports = ( "LICENSE" )
    exports_sources = ( "CMakeLists.txt",
                        "cmake/*",
                        "include/*",
                        "test/*",
                        "LICENSE")

    # Settings
    options = {}
    default_options = {}
    generators = "cmake"

    # Dependencies
    build_requires = ("Catch2/2.7.1@catchorg/stable")

    def package(self):
        cmake = CMake(self)
        cmake.definitions["NOT_NULL_COMPILE_UNIT_TESTS"] = "ON"
        cmake.configure()

        # Compile and run the unit tests
        cmake.build()
        cmake.build(target="test")

        cmake.install()

        self.copy(pattern="LICENSE", dst="licenses")

    def package_id(self):
        self.info.header_only()
