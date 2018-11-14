#include <carma.h>
#include <wyrm>

namespace py = pybind11;

void declare_naga_timer(py::module &mod) {
  py::class_<carma_timer>(mod, "naga_timer")
      .def(py::init())
      .def_property_readonly("total_time",
                             [](carma_timer &ct) { return ct.elapsed(); })
      .def("reset", &carma_timer::reset)
      .def("start", &carma_timer::start)
      .def("stop", &carma_timer::stop);

  py::class_<carma_clock>(mod, "naga_clock")
      .def(py::init([](carma_context &context, int i) {
             return std::unique_ptr<carma_clock>(new carma_clock(&context, i));
           }),
           R"pbdoc(
        Create a carma_clock object which provides timing based on GPU clock count

        Parameters
        ------------
        context: (carma_context): carma context
        i: (int): time buffer size
      )pbdoc",
           py::arg("context"), py::arg("i"))

      .def_property_readonly("timeBuffer",
                             [](carma_clock &clk) { return clk.timeBuffer; })
      .def_property_readonly("cc", [](carma_clock &clk) { return clk.cc; })
      .def_property_readonly("GPUfreq",
                             [](carma_clock &clk) { return clk.GPUfreq; })

      .def("tic", &carma_clock::tic)
      .def("toc", &carma_clock::toc);
}