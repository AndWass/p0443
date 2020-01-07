from conans import ConanFile

class P0443Conan(ConanFile):
    name = "P0443"
    options = {"build_tests": [True, False]}
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    def configure(self):        
        if self.options.build_tests:
            self.requires("doctest/2.3.5")
            self.requires("boost/[>=1.71]@conan/stable")
            self.generators.append("cmake")
        else:
            self.settings.clear()