function res = diff_norm(x)
res = x;
nn = size(res);
for i = 1:nn(1)
    for j = 1:nn(2)
        while (res(i,j) <= -180)
            res(i,j) = res(i,j) + 360;
        end;
        while (res(i,j) > 180)
            res(i,j) = res(i,j) - 360;
        end;
    end;
end;