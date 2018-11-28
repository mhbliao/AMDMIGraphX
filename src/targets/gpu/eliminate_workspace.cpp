#include <migraphx/gpu/eliminate_workspace.hpp>
#include <migraphx/gpu/hip.hpp>
#include <migraphx/program.hpp>
#include <migraphx/instruction.hpp>
#include <migraphx/operators.hpp>
#include <migraphx/iterator_for.hpp>
#include <migraphx/ranges.hpp>
#include <migraphx/stringutils.hpp>
#include <migraphx/pass_config.hpp>

namespace migraphx {
inline namespace MIGRAPHX_INLINE_NS {
namespace gpu {

void eliminate_workspace::apply(program& p) const
{
    if(!enabled(MIGRAPHX_DISABLE_MEMORY_COLORING{}))
        return;

    std::size_t n = 0;
    std::vector<instruction_ref> allocs;
    for(auto ins : iterator_for(p))
    {
        if(ins->outputs().size() != 1)
            continue;
        if(ins->name() != "hip::allocate")
            continue;
        auto&& a = any_cast<hip_allocate>(ins->get_operator());
        if(a.tag == "workspace")
        {
            n = std::max(n, ins->get_shape().bytes());
            allocs.push_back(ins);
        }
    }
    auto ws = p.add_parameter("workspace", shape{shape::int8_type, {n}});
    for(auto&& a : allocs)
    {
        p.replace_instruction(a, ws);
        p.remove_instruction(a);
    }
}

} // namespace gpu
} // namespace MIGRAPHX_INLINE_NS
} // namespace migraphx
