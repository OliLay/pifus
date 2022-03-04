from benchmarks import framework


def start_measurement():
    """Three pifus writers (tap0), three lwIP readers (tap1-3)"""
    framework.start_stack()

    lwip_reader_path = framework.get_binary_path(
        "lwip/custom/benchmark/reader/reader")
    framework.start_process(lwip_reader_path, "192.168.1.202", tapif=1)
    framework.start_process(lwip_reader_path, "192.168.1.203", tapif=2)
    framework.start_process(lwip_reader_path, "192.168.1.204", tapif=3)

    pifus_writer_path = framework.get_binary_path(
        "api/benchmarks/writer/pifus_writer")
    framework.start_process(pifus_writer_path, "192.168.1.202 -p 13337")
    framework.start_process(pifus_writer_path, "192.168.1.203 -p 13337")
    framework.start_process(pifus_writer_path, "192.168.1.204 -p 13337")

    # TODO:
    #   - args for lwIP reader (IP)
    #   - write csv in writer (e2e latency, enqueue until dequeue)


if __name__ == "__main__":
    start_measurement()
