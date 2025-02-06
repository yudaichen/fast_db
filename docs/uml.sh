# 生成类图到 output.puml
clang-uml generate -p compile_commands.json -o doc/output.puml

# 生成时序图到 sequence.puml
clang-uml generate -p compile_commands.json --sequence -o doc/sequence.puml

# 生成包含图到 include.puml
clang-uml generate -p compile_commands.json --include -o doc/include.puml

# 生成包图到 package.puml
clang-uml generate -p compile_commands.json --package -o doc/package.puml