package com.physi.dev.nursinglight.list;

import android.annotation.SuppressLint;
import android.graphics.Typeface;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.physi.dev.nursinglight.R;

import java.util.LinkedList;
import java.util.List;

public class WiFiItemAdapter extends RecyclerView.Adapter<WiFiItemHolder> {
    private List<ScanResult> scanResults = new LinkedList<>();
    private int selectItemPosition = -1;

    @NonNull
    @Override
    public WiFiItemHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        LayoutInflater inflater = LayoutInflater.from(parent.getContext());
        View view = inflater.inflate(R.layout.view_wifi_item, parent, false);
        return new WiFiItemHolder(view);
    }

    @SuppressLint("NotifyDataSetChanged")
    @Override
    public void onBindViewHolder(@NonNull WiFiItemHolder holder, @SuppressLint("RecyclerView") int position) {
        final ScanResult scanResult = scanResults.get(position);

        holder.tvName.setText(scanResult.SSID);
        holder.tvName.setTypeface(null,
                selectItemPosition != position ? Typeface.NORMAL : Typeface.BOLD);

        holder.tvName.setOnClickListener(view -> {
            selectItemPosition = position;
            notifyDataSetChanged();
        });
    }

    @Override
    public int getItemCount() {
        return scanResults.size();
    }

    @SuppressLint("NotifyDataSetChanged")
    public void setItem(List<ScanResult> scanResults){
        this.scanResults.clear();
        for(ScanResult result : scanResults){
            if(result.SSID != null && result.SSID.length() != 0)
                this.scanResults.add(result);
        }
        selectItemPosition = -1;
        notifyDataSetChanged();
    }

    public String getSelectedSSID(){
        if(selectItemPosition == -1)
            return null;
        return scanResults.get(selectItemPosition).SSID;
    }
}
