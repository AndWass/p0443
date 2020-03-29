from conans import ConanFile, CMake, tools

class P0443Conan(ConanFile):
    name = "p0443"
    options = {"build_tests": [True, False]}
    default_options = {"build_tests": False}
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    version = "0.0.4i"
    license = "BSL-1.0"
    exports_sources = "include/*", "LICENSE"
    no_copy_source = True

    def requirements(self):
        if self.options.build_tests:
            self.requires("doctest/2.3.5")
            self.requires("boost/[>=1.71]@conan/stable")
            self.generators.append("cmake")

    def package(self):
        self.copy("*.hpp")
        self.copy("LICENSE", dst="licenses")

    def package_id(self):
        self.info.header_only()
