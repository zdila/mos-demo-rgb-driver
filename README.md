# RGB LED Firmware

Presentation: https://docs.google.com/presentation/d/1dIXqMXzXAHo3agcn2OgOQ0g5IyxKvipEKg0eU7FG1QA/edit?usp=sharing

Webapp: https://github.com/zdila/mos-demo-rgb-webapp

## Installing building and flashing

1. Download and install [Mongoose OS](https://mongoose-os.com/docs/mongoose-os/quickstart/setup.md)
1. Optionally install Docker if you want to build locally
1. Clone the project with `git clone https://github.com/zdila/mos-demo-rgb-driver.git`
1. Go to project directory: `cd mos-demo-rgb-driver`
1. Build with `mos build --platform esp32` or `mos build --platform esp32 --local` for local build
1. Flash your connected device with `mos flash`
1. Configure wifi with `mos wifi SSID PASSWORD`
1. Read device logs with `mos console`
