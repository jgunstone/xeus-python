/***************************************************************************
* Copyright (c) 2018, Martin Renou, Johan Mabille, Sylvain Corlay, and     *
* Wolf Vollprecht                                                          *
* Copyright (c) 2018, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <cmath>
#include <cstdlib>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

#include "xeus/xcomm.hpp"
#include "xeus/xsystem.hpp"

#include "pybind11_json/pybind11_json.hpp"

#include "pybind11/pybind11.h"
#include "pybind11/eval.h"

#include "xtl/xhash.hpp"

#include "xeus-python/xutils.hpp"

#ifdef WIN32
#include "Windows.h"
#endif

namespace py = pybind11;
namespace nl = nlohmann;

namespace xpyt
{
    bool is_pyobject_true(const py::object& obj)
    {
        return PyObject_IsTrue(obj.ptr());
    }

    bool holding_gil()
    {
        return PyGILState_Check();
    }

    void exec(const py::object& code, const py::object& scope)
    {
        // Workaround for https://github.com/pybind/pybind11/issues/1654
        if (scope.attr("get")("__builtins__").is_none())
        {
            scope["__builtins__"] = py::module::import("builtins");
        }
        py::exec("exec(_code_, _scope_, _scope_)", py::globals(), py::dict(py::arg("_code_") = code, py::arg("_scope_") = scope));
    }

    py::object eval(const py::object& code, const py::object& scope)
    {
        // Workaround for https://github.com/pybind/pybind11/issues/1654
        if (scope.attr("get")("__builtins__").is_none())
        {
            scope["__builtins__"] = py::module::import("builtins");
        }
        return py::eval(code, scope);
    }

    std::size_t get_hash_seed()
    {
        return static_cast<std::size_t>(0xc70f6907UL);
    }

    std::string get_tmp_prefix()
    {
        static std::string tmp_prefix = xeus::get_temp_directory_path()
                                      + "/xpython_"
                                      + std::to_string(xeus::get_current_pid())
                                      + '/';
        return tmp_prefix;
    }

    std::string get_tmp_suffix()
    {
        return ".py";
    }

    std::string get_cell_tmp_file(const std::string& content)
    {
        std::uint32_t seed = static_cast<uint32_t>(get_hash_seed());
        std::string id = std::to_string(xtl::murmur2_x86(content.data(), content.size(), seed));
        return get_tmp_prefix() + id + get_tmp_suffix();
    }
}
