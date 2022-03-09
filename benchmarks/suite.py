from priorities import competing
from dummy_stack import dummy

if __name__ == "__main__":
    dummy.measure()
    dummy.draw_plots()

    competing.measure()
    competing.draw_plots()
    exit(0)