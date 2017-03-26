package com.kewldevs.sathish.smartbin;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

import java.text.SimpleDateFormat;
import java.util.Date;

public class ActivityViewBIn extends AppCompatActivity {

    TextView tvBinId,tvBinTime,tvBinLevel;
    String key;
    DatabaseReference mBinRef;
    ProgressBar pbBin;
    ValueEventListener valueEventListener = new ValueEventListener() {

        @Override
        public void onDataChange(DataSnapshot dataSnapshot) {
            Bin b = dataSnapshot.getValue(Bin.class);
            tvBinTime.setText(convertTime(b.getTimestamp()));
            pbBin.setProgress(Integer.parseInt(b.getLevel()));
            tvBinLevel.setText(b.getLevel()+" cm");
        }

        @Override
        public void onCancelled(DatabaseError databaseError) {

        }

    };


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_view_bin);
        tvBinId = (TextView) findViewById(R.id.tvBinId);
        tvBinLevel = (TextView) findViewById(R.id.tvBinLevel);
        tvBinTime = (TextView) findViewById(R.id.tvBinTime);
        pbBin = (ProgressBar) findViewById(R.id.pbBinLevel);
        pbBin.setMax(18);
        pbBin.setProgress(0);
        key = getIntent().getExtras().getString("key");
        if(key!=null)
        {
            tvBinId.setText(key);
            mBinRef = FirebaseDatabase.getInstance().getReference("bins").child(key);
            mBinRef.addValueEventListener(valueEventListener);
        }


    }


    @Override
    protected void onDestroy() {
        super.onDestroy();
        if(mBinRef!=null && valueEventListener!=null) mBinRef.removeEventListener(valueEventListener);
    }


    public static String convertTime(Long unixtime) {
        Date dateObject = new Date(unixtime);
        SimpleDateFormat dateFormatter = new SimpleDateFormat("dd-MM-yy hh:mm aa");
        return dateFormatter.format(dateObject);
    }


}
