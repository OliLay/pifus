import framework
import time

def start_measurement():
    """Three pifus writers (tap0), three lwIP readers (tap1-3)"""
    framework.start_stack()

    lwip_reader_path = framework.get_binary_path(
        "lwip/custom/benchmark/reader/reader")
    framework.start_process(lwip_reader_path, args="192.168.1.201", tapif=1)
   # framework.start_process(lwip_reader_path, args="192.168.1.202", tapif=1)
    #framework.start_process(lwip_reader_path, args="192.168.1.203", tapif=1)

    pifus_writer_path = framework.get_binary_path(
        "api/benchmarks/writer/pifus_writer")
    framework.start_process(pifus_writer_path, args="192.168.1.201 -p 11337 -l HIGH")
    #framework.start_process(pifus_writer_path, args="192.168.1.202 -p 11337 -l MEDIUM")
    #framework.start_process(pifus_writer_path, args="192.168.1.203 -p 11337 -l LOW")

    time.sleep(3)
    framework.kill_all_processes()

    print(framework.compute_latency_from_stamps("write.csv", "written.csv"))

    # TODO: parse write and written and compute latency and check if legit


