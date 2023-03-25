import os

Import("env")

test_string = os.getenv("TEST_STRING", 'default')

print('{} \n'.format(test_string))

env.Append(CPPDEFINES=[
    ("ENV_TEST_STRING", env.StringifyMacro(test_string)),
])
