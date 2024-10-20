#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // For STL types like std::vector
#include <iostream>

namespace py = pybind11;

// Expose the NLPModel to Python
PYBIND11_MODULE(my_module, m) {
    m.doc() = "pybind11 module for NLPModel"; // Optional module docstring

    // Import the spacy module from Python
    py::module spacy = py::module::import("spacy");

    // Define the NLPModel class
    py::class_<NLPModel>(m, "NLPModel")
        .def(py::init<>()) // Constructor
        .def("process_text", &NLPModel::process_text, "Process text using spaCy NLP")
        .def("compare_similarity", &NLPModel::compare_similarity, "Compare similarity between a question and answers");
}