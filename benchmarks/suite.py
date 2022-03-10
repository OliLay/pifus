import priorities.competing
import dummy_stack.multiple_async
import dummy_stack.multiple_sync
import dummy_stack.single

if __name__ == "__main__":
    dummy_stack.multiple_async.measure()
    dummy_stack.multiple_async.draw_plots()

    dummy_stack.multiple_sync.measure()
    dummy_stack.multiple_sync.draw_plots()

    dummy_stack.single.measure()
    dummy_stack.single.draw_plots()

    priorities.competing.measure()
    priorities.competing.draw_plots()
    exit(0)