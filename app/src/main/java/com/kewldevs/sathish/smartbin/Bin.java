
package com.kewldevs.sathish.smartbin;




public class Bin {

    private String mLevel;
    private Location mLocation;
    private Long mTimestamp;

    Bin()
    {

    }

    public String getLevel() {
        return mLevel;
    }

    public void setLevel(String level) {
        mLevel = level;
    }

    public Location getLocation() {
        return mLocation;
    }

    public void setLocation(Location location) {
        mLocation = location;
    }

    public Long getTimestamp() {
        return mTimestamp;
    }

    public void setTimestamp(Long timestamp) {
        mTimestamp = timestamp;
    }

    @Override
    public String toString() {
        return "Bin{" +
                "mLevel='" + mLevel + '\'' +
                ", mTimestamp=" + mTimestamp +
                '}';
    }
}
