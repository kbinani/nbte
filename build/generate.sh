(

set -ue

cd "$(dirname "$0")"

cmake .. -G Xcode
bundle exec pod install

# This second "cmake .." is necessary to avoid link error against Bugsnag idn why
cmake ..

)
