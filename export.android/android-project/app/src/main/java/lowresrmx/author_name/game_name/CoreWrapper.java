package lowresrmx.author_name.game_name;

public class CoreWrapper {
    public interface Listener {
        void onControlsDidChange(ControlsInfo info);
    }

    private final Listener listener;

    public CoreWrapper(Listener listener) {
        this.listener = listener;
        nativeRegister();
    }

    // Called from JNI
    private void onControlsDidChange(int keyboardMode, int hapticMode, boolean isAudioEnabled, boolean isInputState, boolean isCompatMode) {
        if (listener != null) {
            listener.onControlsDidChange(new ControlsInfo(keyboardMode, hapticMode, isAudioEnabled, isInputState, isCompatMode));
        }
    }

    private native void nativeRegister();
}
