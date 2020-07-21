#include <cryptofuzz/options.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <iostream>
#include <stdlib.h>
#include <cryptofuzz/repository.h>

namespace cryptofuzz {

Options::Options(const int argc, char** argv) {
    for (int i = 0; i < argc; i++) {
        arguments.push_back( std::string(argv[i]) );
    }

    for (size_t i = 1; i < arguments.size(); i++) {
        const auto arg = arguments[i];
        std::vector<std::string> parts;
        boost::split(parts, arg, boost::is_any_of("="));

        if ( arg == "--debug" ) {
            debug = true;
        } else if ( !parts.empty() && parts[0] == "--operations" ) {
            if ( parts.size() != 2 ) {
                std::cout << "Expected argument after --operations=" << std::endl;
                exit(1);
            }

            std::vector<std::string> operationStrings;
            boost::split(operationStrings, parts[1], boost::is_any_of(","));

            std::vector<uint64_t> operationIDs;

            for (const auto& curOpStr : operationStrings) {
                bool found = false;
                for (size_t i = 0; i < (sizeof(repository::OperationLUT) / sizeof(repository::OperationLUT[0])); i++) {
                    if ( boost::iequals(curOpStr, std::string(repository::OperationLUT[i].name)) ) {
                        operationIDs.push_back(repository::OperationLUT[i].id);
                        found = true;
                        break;
                    }
                }

                if ( found == false ) {
                    std::cout << "Undefined operation: " << curOpStr << std::endl;
                    exit(1);
                }
            }

            this->operations = operationIDs;
        } else if ( !parts.empty() && parts[0] == "--force-module" ) {
            if ( parts.size() != 2 ) {
                std::cout << "Expected argument after --operations=" << std::endl;
                exit(1);
            }

            const auto& moduleStr = parts[1];

            bool found = false;
            uint64_t forceModule;
            for (size_t i = 0; i < (sizeof(repository::ModuleLUT) / sizeof(repository::ModuleLUT[0])); i++) {
                if ( boost::iequals(moduleStr, std::string(repository::ModuleLUT[i].name)) ) {
                    forceModule = repository::ModuleLUT[i].id;
                    found = true;
                    break;
                }
            }

            if ( found == false ) {
                std::cout << "Undefined module: " << moduleStr << std::endl;
                exit(1);
            }

            this->forceModule = forceModule;
        }
    }
}

} /* namespace cryptofuzz */
