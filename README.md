![Logo](https://github.com/iotoasis/SO/blob/master/logo_oasis_m.png)

# Oasis Project

본 Oasis (Open-source Architecture Semantic Iot Service-platform) 프로젝트는 국제 표준을 준용하는 오픈 소스 기반 지능형 사물 인터넷 서비스 플랫폼을 개발하는 것을 목표로 하고 있습니다.

본 Oasis 프로젝트는 2015년도 정보통신/방송 기술개발사업 신규지원 대상과제 "(ICBMS-3세부) 사물 가상화, 분산 자율지능 및 데이터 연계/분석을 지원하는 IoT 기반 플랫폼 기술 개발" 과제의 결과물로써 오픈소스로 제공됩니다.

본 Oasis 프로젝트는 오픈 소스 커뮤니티를 기반으로 오픈소스로써 계속적으로 성장해 나갈 계획입니다.

# DGW (Device Gateway)
 DGW는 크게 데이터를 수집하고 특정 기능을 수행하는 Sensor/Device와<br>
 Sensor/Device로 부터 수집된 데이터를 SI Server로 정보를 주고받는 역할을 수행하는 Gateway(Hub)로 나뉜다.<br>
 ※ 이하, 데이터 수집 기능만 가지는 것을 Sensor, 데이터 수집 및 특정 기능을 실행할 수 있는 것을 Device라 부른다.


## Gateway(Hub)
 Gateway의 HW는 Raspberry Pi3 Model B를 사용하며, OS는 Raspbian(Debian 계열 Linux)이다.<br>
 Gateway는 Sensor/Device로 부터 데이터를 수집하고 이를 가공하여 SI Server로 전달하는 Report 기능과<br>
 SI Server로 부터 제어 메시지를 받아서 Device를 제어하는 Control 기능을 가진다.
 

## Sensor/Device
 Grib의 Sensor/Device의 HW는 Aduino 계열(+CC2541 BLE Module)을 사용한다.<br>
 Gateway와 BLE 통신중에 Sensor/Device가 Slave Mode로 동작하여, 일정 주기로 데이터를 전달한다.<br>
 <br>
 한양대에서 독자적으로 개발한 센서에 대한 설명은 하기 링크를 참조.<br>
 [한양대 센서 설명](./HY_SENSOR/Readme_HY.md)<br>


## Documents
 - [SetUp Guide](./GRIB_DOC/SetupGuide.md)
 - [Build Guide](./GRIB_DOC/BuildGuide.md)
 - [Gateway Guide](./GRIB_DOC/GatewayGuide.md)


## Downloads
 - [Latest Release](https://github.com/iotoasis/DGW/releases/)


## License
 Licensed under the BSD License, Version 2.0
<br>