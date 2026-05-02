#pragma once
#include <string>
#include <vector>


static const std::vector<std::string> texts{
    "The Cake is a LIE", "FCK STG", "You're mom loves you!", "<>Mehrzweckeier<>", "Umfahren != Umfahren",
    "403, don't feel like it rn...", "Hamsters survive blenders! (maybe)", "Einfach Orangensaft!", "Ha! Nerd!",
    "sigsegv at line -23,69", "Error? What erorr?", "Now with K!", "t.84 says I have priority!", "Ouch! My Bones!",
    "CODE 418 IS BRITISH!", "Franzosen GRRRRR", "Ready? Or not?", "MLO wants to know your location.",
    "Did you shower today?", "Patch 1.04.02a turned the frogs gay.", "Brennholzverleih!", "'Chill game!' -Andy84"
};


inline std::string GetSplashText() {
    static int i = 0;
    return texts.at(i++ % texts.size());
}
