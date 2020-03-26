function res = my_error(x,y)
try
    res = sqrt(sum(sum(diff_norm(x - y).^2)))/length(x);
catch
    x = x';
    res = sqrt(sum(sum(diff_norm(x - y).^2)))/length(x);
end;