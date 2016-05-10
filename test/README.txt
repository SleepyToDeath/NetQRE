To support running of multiple apps in the NetQRE environment, please follow
the following steps to first compile the runtime and then deploy it on mininet
if desired.

Step 1) Compile "main_prog.cpp" to generate "runtime" binary using the command:
        g++ main_prog.cpp -std=c++11 -lpthread -g -o runtime

Step 2) Run the binary "main" on the terminal if you want to conduct an
        offline monitoring using the command:
        ./runtime

Note:   The runtime starts and waits for you to enter the app you want to
        start. The command remains the same as you would do in case of a
        normal app execution.

Step 3) Enter the command to start the app. The app will start and
        notify you its name.

Step 4) If you would like to stop an app that is already running, run the
        command:
        stop <app_name>
        where <app_name> is the name of teh app that was notified to you when
        you started it.

For deploying it on mininet:
Once you start mininet using the script setup_mirror.py, run the command:
h4 ./runtime
to start the runtime binary and the steps remain the same as above except that
the mode would change to "live" and the device to be sniffed to "h4-eth0".

Step 5) If the runtime is to be stopped, execute:
        stop
        on the runtime terminal to stop it.
