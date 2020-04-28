from conans import ConanFile, CMake, tools

class P0443Conan(ConanFile):
    name = "p0443"
    requires = ["boost/[>=1.72]"]

    options = {"build_tests": [True, False]}
    default_options = {"build_tests": False}
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    version = "0.0.10"
    license = "BSL-1.0"
    exports_sources = "include/*", "LICENSE"
    no_copy_source = True

    def requirements(self):
        if self.options.build_tests:
            self.requires("doctest/2.3.5")
            self.generators.append("cmake")

    def package(self):
        self.copy("*.hpp")
        self.copy("LICENSE", dst="licenses")

    def package_id(self):
        self.info.header_only()
