#include <string>
#include <vector>

#include "core/program.h"

struct VerifyError {
    size_t pc;            // адрес команды; для ошибок всей программы — 0
    std::string message;
};

class Verifier {
public:
    explicit Verifier(const Program &program);

    // запускает все проверки; пустой вектор - программа корректна
    std::vector<VerifyError> verify();

private:
    const Program &program;
    std::vector<VerifyError> errors;

    void error(size_t pc, const std::string &message); // добавить ошибку
    void checkProgram(); // проверки всей программы (6-7)
    void checkInstruction(size_t pc, const Instruction &ins); // проверки одной команды (1-5)
};
