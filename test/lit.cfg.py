import os

import lit.formats

from lit.llvm import llvm_config
from lit.llvm.subst import ToolSubst

config.name = "bf-jit"
config.test_format = lit.formats.ShTest(not llvm_config.use_lit_shell)
config.suffixes = [".bf"]
config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = os.path.join(config.bf_obj_root, "test")

config.llvm_tools_dir = config.bf_tools_dir

tool_substitutions = [
    ToolSubst("bf-translate", unresolved="fatal"),
    ToolSubst("bf-opt", unresolved="fatal"),
]

llvm_config.add_tool_substitutions(tool_substitutions)