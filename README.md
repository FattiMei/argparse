# argparse
Single file, single header C++ library for primitive command line argument parsing. Inspired by python argparse library

## Motivation
Command line arguments allow changes in the program configuration without a recompilation cycle. This improves my experience in profiling programs across multiple (non-local) machines and program options.

For a typical numerical code, one could control:
  * working precision (`float32` vs `float64`)
  * problem size
  * solver parameters (number of iterations, solver type)

I reckon this is a cleaner interaction than using config files.


## Inspiration
This is heavily inspired by [CLI11](github.com/CLIUtils/CLI11) and I particularly like the "reference capture" approach.

```c++
#include <CLI/CLI.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
    CLI::App app{"Compute with 3 optional parameters"};

    double p1=0, p2=0, p3=0;
    app.add_option("--p1", p1, "First parameter");
    app.add_option("--p2", p2, "Second parameter");
    app.add_option("--p3", p3, "Third parameter");

    CLI11_PARSE(app, argc, argv);

    std::cout << "p1=" << p1 << " p2=" << p2 << " p3=" << p3 << "\n";
}
```

The CLI11 library is a complete solution for command line argument parsing, but it's not a lightweight dependency. The single header artifact stands at over 12000 LOC. I want to produce a simpler implementation (of course some features will be missing)


## Variable binding
The core of the problem lies in retrieving variables after a successfull command line argument parsing. When arguments are registered by their name, it would be nice to access them using the same name. This is particularly difficult in statically typed languages, and the solution below (from the [cxxopts](https://github.com/jarro2783/cxxopts) library) relies on dictionaries and the ability of the user to remember the correct type in a cast.

```c++
#include <cxxopts.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
    cxxopts::Options options("myprog", "Compute with 3 optional parameters");
    options.add_options()
        ("p1", "first param", cxxopts::value<double>()->default_value("0"))
        ("p2", "second param", cxxopts::value<double>()->default_value("0"))
        ("p3", "third param", cxxopts::value<double>()->default_value("0"))
        ("h,help", "Print usage");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << "\n";
        return 0;
    }

    std::cout << "p1=" << result["p1"].as<double>() << "\n";
}
```

Capturing the references solves this problem in an elegant way. The drawback is wanting a borrow checker to enforce there is only one mutable reference at one moment, maybe I'll get back to this.


## General understanding of the problem
An argument could be either:
  * a **flag**: `ls -l`
  * an **option**: `gcc -o out`
  * a **positional**, which is required: `grep "pattern"

when entering a new argument, one would have to satisfy some constraints on the argument name:
  * a flag argument can be only of boolean type
  * a flag or positional must be like `-f`, `-foo` or `--use-float32`
  * a positional must be an identifier, it can't start with `-`


## Proposed design
```c++
class ArgumentParser {
    public:
        ArgumentParser(std::string name, std::string description);

        void add_flag(std::string name, bool& ref, std::string description);

        template <typename T>
        void add_optional(std::string name, T& ref, std::string description);

        template <typename T>
        void add_positional(std::string name, T& ref, std::string description);

        bool parse_args(int argc, char* argv[]);
};
```

with runtime checks on the argument names.
