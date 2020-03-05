echo y | del "%1\Debug Dynamic"
rmdir "%1\Debug Dynamic"
echo y | del "%1\Debug Static"
rmdir "%1\Debug Static"
echo y | del "%1\Debug Single"
rmdir "%1\Debug Single"
echo y | del "%1\Release Dynamic"
rmdir "%1\Release Dynamic"
echo y | del "%1\Release Static"
rmdir "%1\Release Static"
echo y | del "%1\Release Single"
rmdir "%1\Release Single"
del %1\*.user

