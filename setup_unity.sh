# Download Unity testing framework
cd /tmp
git clone https://github.com/ThrowTheSwitch/Unity.git
cd - # Return to myshell directory
mkdir -p tests/unity
cp /tmp/Unity/src/unity.c /tmp/Unity/src/unity.h /tmp/Unity/src/unity_internals.h tests/unity/

# Unity integration complete - files organized under tests/unity/
