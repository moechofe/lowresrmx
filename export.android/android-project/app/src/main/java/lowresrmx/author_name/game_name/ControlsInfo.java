package lowresrmx.author_name.game_name;

public class ControlsInfo {
    public final KeyboardMode keyboardMode;
    public final HapticMode hapticMode;
    public final boolean isAudioEnabled;
    public final boolean isInputState;
    public final boolean isCompatMode;

    public ControlsInfo(int keyboardMode, int hapticMode, boolean isAudioEnabled, boolean isInputState, boolean isCompatMode) {
        this.keyboardMode = KeyboardMode.values()[keyboardMode];
        this.hapticMode = HapticMode.values()[hapticMode];
        this.isAudioEnabled = isAudioEnabled;
        this.isInputState = isInputState;
        this.isCompatMode = isCompatMode;
    }
}
