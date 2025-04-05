package criminal.russia.custom.main.tab;

public class PlayerData {

    private String mName;

    private int mID;
    private int mLevel;
    private int mPing;

    public PlayerData(int id, String name, int level, int ping) {
        this.mID = id;
        this.mName = name;
        this.mLevel = level;
        this.mPing = ping;
    }

    public int getId() {
        return mID;
    }

    public String getName() {
        return mName;
    }

    public int getPing() {
        return mPing;
    }

    public int getLevel() {
        return mLevel;
    }

    public void setId(int id) {
        this.mID = id;
    }

    public void setName(String name) {
        this.mName = name;
    }

    public void setPing(int ping) {
        this.mPing = ping;
    }

    public void setLevel(int level) {
        this.mLevel = level;
    }
}
