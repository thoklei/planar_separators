/**
 * This file serves as a workbench to try things out.
 */
#include <iostream>

class SuperClass {

public:
    SuperClass() {
        std::cout << "Calling Superclass Constructor" << std::endl;
    }

    virtual void interFaceMethod() {
        std::cout << "Calling Superclass interface method" << std::endl;
        apply();
        std::cout << "Done calling apply" << std::endl;
    }

    virtual std::string getName() const {
        std::cout << "In superclass" << std::endl;
        std::string name = getName();
        return name + "_hahaItWorked";
    }

protected:

    virtual void apply() = 0;
};

class InheritingClass : public SuperClass {

public:
    InheritingClass() {
        std::cout << "Calling Inheriting Constructor" << std::endl;
    }

    void apply() override {
        std::cout << "Calling actual apply-method" << std::endl;
    }

    std::string getName() const override {
        std::cout << "In inheriting class" << std::endl;
        return "Inheriting_BOI";
    }
};


int main() {

    InheritingClass inh;

    inh.interFaceMethod();
    std::cout << inh.getName();

    return 0;
}